#include "ClientRepo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/File.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

/// Statuses of table items.
/** Should be in sync with values in SQL statements. */
struct ClientRepo::ItemStatuses
{
	enum _
	{
		/// server version in case of no conflict
		server,
		/// client change based on 'server' in case of no conflict
		client,
		/// client change based on 'server' which is in process of committing to server
		transient,
		/// client change based on 'transient', waiting for results of commit of 'transient'
		postponed,
		/// old version of data, for reference only, in case of conflict
		conflictBase,
		/// client version in case of conflict
		conflictClient,

		_count
	};
};

struct ClientRepo::KeyItems
{
	long long ids[ItemStatuses::_count];

	KeyItems()
	{
		for(int i = 0; i < ItemStatuses::_count; ++i)
			ids[i] = 0;
	}
};

/*

Possible combinations of items with the same key:

* no data
	<nothing>
* just unchanged data
	server
* client added new data, which wasn't on server before
	client
* new added client data is committing to server
	transient
* client changed new data, while previous change is still committing
	transient postponed
* new data conflicted because someone added new data too
	server conflictClient
* new data conflicted, but server version isn't known yet
	conflictClient
* client changed data
	server client
* changed data is commiting to server
	server transient
* client changed data, while previous change is still committing
	server transient postponed
* changed data conflicted because someone changed this data too
	conflictBase server conflictClient
* changed data conflicted but server version isn't known yet
	conflictBase conflictClient

Sync includes two substeps - push and pull.
Push tryes to commit client changes on server.
Pull gets server changes to client.

Algorithm of sync.

Request:
* Select subset of 'client' changes satisfying constraints.
* Transform these 'client's into 'transient's.
* Send keys and values of the subset in order.
Response:
* Receive numbers of succeeded client changes.
	* Number that missing means corresponding change was conflicted.
* For each successful change:
	* 'transient' becomes 'server', old 'server' (if presents) disappears
	* 'postponed' (if presents) becomes 'client'
* For each conflicted change:
	* if 'postponed' presents, it becomes 'conflictClient'
	* if 'postponed' presents, 'transient' disappears
	* if 'postponed' doesn't present, 'transient' becomes 'conflictClient'
	* if 'server' presents, it becomes 'conflictBase'
* Receive server changes.
* For each server change:
	* new data becomes 'server', old 'server' (if presents) disappears

Note that new 'server' values from successful change and from server change
don't intersect - otherwise it would be conflict.

In case of some non-conflict failure (i.e. network problem),
database should be cleaned up:
* if 'postponed' presents, it becomes 'client'
* if 'postponed' presents, 'transient' disappears
* if 'postponed' doesn't present, 'transient' becomes 'client'

Cleanup procedure should run when database is opened.

Note that conflict could be without 'server'. It happens when new server value
is not arrived yet with pull. Server knew it is conflict, so it didn't
allow commit, but server always sends updates in order of revisions.
Client can't resolve such a conflict until update is arrived.

*/

ClientRepo::ClientRepo(const char* fileName)
: Repo(fileName)
{
	BEGIN_TRY();

	// create table items
	if(sqlite3_exec(*db,
		"CREATE TABLE IF NOT EXISTS items ("
		"id INTEGER PRIMARY KEY, "
		"value BLOB, " // value comes first to decrease moving large blob
		"key BLOB NOT NULL, "
		"status INTEGER NOT NULL, " // see ItemStatus enum
		"rev INTEGER NOT NULL)", // revision of 'server' or 'conflictBase' item, zero for others
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create table items", db->Error());
	// create index items_key_status
	if(sqlite3_exec(*db,
		"CREATE UNIQUE INDEX IF NOT EXISTS items_key_status ON items (key, status)",
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create index items_key_status", db->Error());

	// create statements

	stmtGetKeyItems = db->CreateStatement("SELECT id, status FROM items WHERE key = ?1");
	stmtAddKeyItem = db->CreateStatement("INSERT OR REPLACE INTO items (key, value, status, rev) VALUES (?1, ?2, ?3, ?4)");
	stmtRemoveKeyItem = db->CreateStatement("DELETE FROM items WHERE id = ?1");
	stmtChangeKeyItemStatus = db->CreateStatement("UPDATE OR REPLACE items SET status = ?2 WHERE id = ?1");
	stmtChangeKeyItemValue = db->CreateStatement("UPDATE items SET value = ?2 WHERE id = ?1");

	END_TRY("Can't create client repo");
}

void ClientRepo::CheckItemStatus(int itemStatus)
{
	if(itemStatus < 0 || itemStatus >= ItemStatuses::_count)
		THROW("DB corruption: invalid item status");
}

ClientRepo::KeyItems ClientRepo::GetKeyItems(ptr<File> key)
{
	Data::SqliteQuery query(stmtGetKeyItems);

	stmtGetKeyItems->Bind(1, key);

	bool done = false;
	KeyItems keyItems;
	while(!done)
	{
		switch(stmtGetKeyItems->Step())
		{
		case SQLITE_ROW:
			{
				int itemStatus = stmtGetKeyItems->ColumnInt(1);
				CheckItemStatus(itemStatus);
				keyItems.ids[itemStatus] = stmtGetKeyItems->ColumnInt64(0);
			}
			break;
		case SQLITE_DONE:
			done = true;
			break;
		default:
			THROW_SECONDARY("Error getting key items", db->Error());
		}
	}

	return keyItems;
}

void ClientRepo::AddKeyItem(ptr<File> key, ptr<File> value, int status, long long revision)
{
	Data::SqliteQuery query(stmtAddKeyItem);

	stmtAddKeyItem->Bind(1, key);
	stmtAddKeyItem->Bind(2, value);
	stmtAddKeyItem->Bind(3, status);
	stmtAddKeyItem->Bind(4, revision);

	if(stmtAddKeyItem->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error adding key item", db->Error());
}

void ClientRepo::RemoveKeyItem(long long itemId)
{
	Data::SqliteQuery query(stmtRemoveKeyItem);

	stmtRemoveKeyItem->Bind(1, itemId);

	if(stmtRemoveKeyItem->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error removing key item", db->Error());
}

void ClientRepo::ChangeKeyItemStatus(long long itemId, int newItemStatus)
{
	Data::SqliteQuery query(stmtChangeKeyItemStatus);

	stmtChangeKeyItemStatus->Bind(1, itemId);
	stmtChangeKeyItemStatus->Bind(2, newItemStatus);

	if(stmtChangeKeyItemStatus->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error changing key item status", db->Error());
}

void ClientRepo::ChangeKeyItemValue(long long itemId, ptr<File> newValue)
{
	Data::SqliteQuery query(stmtChangeKeyItemValue);

	stmtChangeKeyItemValue->Bind(1, itemId);
	stmtChangeKeyItemValue->Bind(2, newValue);

	if(stmtChangeKeyItemValue->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error changing key item value", db->Error());
}

void ClientRepo::Change(ptr<File> key, ptr<File> value)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	// get acquainted with this key
	KeyItems keyItems = GetKeyItems(key);

	// select status for the change
	int status;
	if(keyItems.ids[ItemStatuses::transient])
		status = ItemStatuses::postponed;
	else if(keyItems.ids[ItemStatuses::conflictClient])
		status = ItemStatuses::conflictClient;
	else
		status = ItemStatuses::client;

	// if item is present, change it
	if(keyItems.ids[status])
		ChangeKeyItemValue(keyItems.ids[status], value);
	// else add new item
	else
		AddKeyItem(key, value, status, 0);

	END_TRY("Can't change repo value");
}

void ClientRepo::Resolve(ptr<File> key)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	// get acquainted with this key
	KeyItems keyItems = GetKeyItems(key);

	// if there is no conflict, this is an error
	if(!keyItems.ids[ItemStatuses::conflictClient])
		THROW("Key is not in conflict");

	// make client value not conflict
	ChangeKeyItemStatus(keyItems.ids[ItemStatuses::conflictClient], ItemStatuses::client);

	// if there is conflict base, remove it
	if(keyItems.ids[ItemStatuses::conflictBase])
		RemoveKeyItem(keyItems.ids[ItemStatuses::conflictBase]);

	END_TRY("Can't resolve repo conflict");
}

END_INANITY_OIL

#include "ClientRepo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/PartFile.hpp"
#include "../inanity/Exception.hpp"
#include <sstream>

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

struct ClientRepo::ManifestKeys
{
	enum _
	{
		/// Global revision.
		/** Guaranteed, that if key is not changed after
		that revision, its value is actual in client repo. */
		globalRevision,

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
: Repo(fileName), pushInProgress(false)
{
	BEGIN_TRY();

	// create table manifest
	if(sqlite3_exec(*db,
		"CREATE TABLE IF NOT EXISTS manifest ("
		"key INTEGER PRIMARY KEY, "
		"value ANY NOT NULL)",
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create table manifest", db->Error());

	// create table items
	if(sqlite3_exec(*db,
		"CREATE TABLE IF NOT EXISTS items ("
		"id INTEGER PRIMARY KEY, "
		"value BLOB NOT NULL, " // value comes first to decrease moving large blob
		"key BLOB NOT NULL, "
		"status INTEGER NOT NULL)", // see ItemStatus enum
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create table items", db->Error());
	// create index items_key_status
	if(sqlite3_exec(*db,
		"CREATE UNIQUE INDEX IF NOT EXISTS items_key_status ON items (key, status)",
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create index items_key_status", db->Error());
	// create index items_status_partial
	{
		std::ostringstream ss;
		ss
			<< "CREATE INDEX IF NOT EXISTS items_status_partial ON items (status) WHERE"
			<< " status = " << ItemStatuses::client
			<< " OR status = " << ItemStatuses::transient
			<< " OR status = " << ItemStatuses::postponed;
		if(sqlite3_exec(*db, ss.str().c_str(), 0, 0, 0) != SQLITE_OK)
			THROW_SECONDARY("Can't create index items_status_partial", db->Error());
	}

	// create statements

	stmtManifestGet = db->CreateStatement("SELECT value FROM manifest WHERE key = ?1");
	stmtManifestSet = db->CreateStatement("INSERT OR REPLACE INTO manifest (key, value) VALUES (?1, ?2)");
	stmtGetKeyItems = db->CreateStatement("SELECT id, status FROM items WHERE key = ?1");
	stmtGetKeyItemsByOneItemId = db->CreateStatement("SELECT id, status FROM items WHERE key = (SELECT key FROM items WHERE id = ?1)");
	stmtGetKeyItemValue = db->CreateStatement("SELECT value FROM items WHERE id = ?1");
	stmtAddKeyItem = db->CreateStatement("INSERT OR REPLACE INTO items (key, value, status) VALUES (?1, ?2, ?3)");
	stmtRemoveKeyItem = db->CreateStatement("DELETE FROM items WHERE id = ?1");
	stmtChangeKeyItemStatus = db->CreateStatement("UPDATE OR REPLACE items SET status = ?2 WHERE id = ?1");
	stmtChangeKeyItemValue = db->CreateStatement("UPDATE items SET value = ?2 WHERE id = ?1");
	{
		std::ostringstream ss;
		ss << "SELECT id, key, value FROM items WHERE status = " << ItemStatuses::client << " LIMIT " << maxPushKeysCount;
		stmtSelectKeysToPush = db->CreateStatement(ss.str().c_str());
	}
	stmtMassChangeStatus = db->CreateStatement("UPDATE OR REPLACE items SET status = ?2 WHERE status = ?1");

	// create helper empty file
	emptyFile = NEW(PartFile(keyBufferFile, keyBufferFile->GetData(), 0));

	END_TRY("Can't create client repo");
}

ptr<Data::SqliteDb> ClientRepo::GetDb() const
{
	return db;
}

void ClientRepo::CheckItemStatus(int itemStatus)
{
	if(itemStatus < 0 || itemStatus >= ItemStatuses::_count)
		THROW("DB corruption: invalid item status");
}

ClientRepo::KeyItems ClientRepo::FillKeyItems(Data::SqliteStatement* stmt)
{
	bool done = false;
	KeyItems keyItems;
	while(!done)
	{
		switch(stmt->Step())
		{
		case SQLITE_ROW:
			{
				int itemStatus = stmt->ColumnInt(1);
				CheckItemStatus(itemStatus);
				keyItems.ids[itemStatus] = stmt->ColumnInt64(0);
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

ClientRepo::KeyItems ClientRepo::GetKeyItems(ptr<File> key)
{
	Data::SqliteQuery query(stmtGetKeyItems);

	stmtGetKeyItems->Bind(1, key);

	return FillKeyItems(stmtGetKeyItems);
}

ClientRepo::KeyItems ClientRepo::GetKeyItemsByOneItemId(long long itemId)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtGetKeyItemsByOneItemId);

	stmtGetKeyItemsByOneItemId->Bind(1, itemId);

	return FillKeyItems(stmtGetKeyItemsByOneItemId);
}

ptr<File> ClientRepo::GetKeyItemValue(long long itemId)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtGetKeyItemValue);

	stmtGetKeyItemValue->Bind(1, itemId);

	if(stmtGetKeyItemValue->Step() != SQLITE_ROW)
		THROW_SECONDARY("Error getting key item value", db->Error());

	ptr<File> result = stmtGetKeyItemValue->ColumnBlob(0);
	if(!result->GetSize())
		return nullptr;
	return result;
}

void ClientRepo::AddKeyItem(ptr<File> key, ptr<File> value, int status)
{
	Data::SqliteQuery query(stmtAddKeyItem);

	stmtAddKeyItem->Bind(1, key);
	stmtAddKeyItem->Bind(2, value);
	stmtAddKeyItem->Bind(3, status);

	if(stmtAddKeyItem->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error adding key item", db->Error());
}

void ClientRepo::RemoveKeyItem(long long itemId)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtRemoveKeyItem);

	stmtRemoveKeyItem->Bind(1, itemId);

	if(stmtRemoveKeyItem->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error removing key item", db->Error());
}

void ClientRepo::ChangeKeyItemStatus(long long itemId, int newItemStatus)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtChangeKeyItemStatus);

	stmtChangeKeyItemStatus->Bind(1, itemId);
	stmtChangeKeyItemStatus->Bind(2, newItemStatus);

	if(stmtChangeKeyItemStatus->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error changing key item status", db->Error());
}

void ClientRepo::ChangeKeyItemValue(long long itemId, ptr<File> newValue)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtChangeKeyItemValue);

	stmtChangeKeyItemValue->Bind(1, itemId);
	stmtChangeKeyItemValue->Bind(2, newValue);

	if(stmtChangeKeyItemValue->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error changing key item value", db->Error());
}

long long ClientRepo::GetManifestValue(int key, long long defaultValue)
{
	THROW_ASSERT(key >= 0 && key < ManifestKeys::_count);

	Data::SqliteQuery query(stmtManifestGet);

	stmtManifestGet->Bind(1, key);

	switch(stmtManifestGet->Step())
	{
	case SQLITE_ROW:
		return stmtManifestGet->ColumnInt64(0);
	case SQLITE_DONE:
		return defaultValue;
	default:
		THROW_SECONDARY("Error getting manifest value", db->Error());
	}
}

void ClientRepo::SetManifestValue(int key, long long value)
{
	THROW_ASSERT(key >= 0 && key < ManifestKeys::_count);

	Data::SqliteQuery query(stmtManifestSet);

	stmtManifestSet->Bind(1, key);
	stmtManifestSet->Bind(2, value);

	if(stmtManifestSet->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error setting manifest value", db->Error());
}

void ClientRepo::Change(ptr<File> key, ptr<File> value)
{
	BEGIN_TRY();

	// check key size
	if(key->GetSize() <= 0)
		THROW("Key length should be positive");

	// replace value with zero file
	if(!value)
		value = emptyFile;

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
		AddKeyItem(key, value, status);

	transaction.Commit();

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

	transaction.Commit();

	END_TRY("Can't resolve repo conflict");
}

ptr<File> ClientRepo::GetValue(ptr<File> key)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	KeyItems keyItems = GetKeyItems(key);

	int order[] = {
		ItemStatuses::postponed,
		ItemStatuses::transient,
		ItemStatuses::client,
		ItemStatuses::conflictClient,
		ItemStatuses::server
	};

	for(int i = 0; i < sizeof(order) / sizeof(order[0]); ++i)
		if(keyItems.ids[order[i]])
			return GetKeyItemValue(keyItems.ids[order[i]]);

	return nullptr;

	END_TRY("Can't get repo value");
}

bool ClientRepo::IsConflicted(ptr<File> key)
{
	BEGIN_TRY();

	KeyItems keyItems = GetKeyItems(key);

	return !!keyItems.ids[ItemStatuses::conflictClient];

	END_TRY("Can't get is repo value conflicted");
}

ptr<File> ClientRepo::GetConflictServerValue(ptr<File> key)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	KeyItems keyItems = GetKeyItems(key);

	if(!keyItems.ids[ItemStatuses::conflictClient])
		THROW("Key is not in conflict");

	if(keyItems.ids[ItemStatuses::server])
		return GetKeyItemValue(keyItems.ids[ItemStatuses::server]);

	return nullptr;

	END_TRY("Can't get repo conflict server value");
}

ptr<File> ClientRepo::GetConflictBaseValue(ptr<File> key)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	KeyItems keyItems = GetKeyItems(key);

	if(!keyItems.ids[ItemStatuses::conflictClient])
		THROW("Key is not in conflict");

	if(keyItems.ids[ItemStatuses::conflictBase])
		return GetKeyItemValue(keyItems.ids[ItemStatuses::conflictBase]);

	return nullptr;

	END_TRY("Can't get repo conflict base value");
}

void ClientRepo::Push(StreamWriter* writer)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	// write our revision
	writer->WriteShortlyBig(GetManifestValue(ManifestKeys::globalRevision, 0));

	// select keys to push
	Data::SqliteQuery query(stmtSelectKeysToPush);

	THROW_ASSERT(transientIds.empty());

	// control total size of push
	size_t totalPushSize = 0;
	bool done = false;
	while(!done)
	{
		switch(stmtSelectKeysToPush->Step())
		{
		case SQLITE_ROW:
			// check total push size
			totalPushSize += stmtSelectKeysToPush->ColumnBlobSize(2);
			if(totalPushSize > maxPushTotalSize)
			{
				// stop push, that's enough
				done = true;
				break;
			}

			// output key
			{
				ptr<File> key = stmtSelectKeysToPush->ColumnBlob(1);
				size_t keySize = key->GetSize();
				writer->WriteShortly(keySize);
				writer->Write(key->GetData(), keySize);
			}
			// output value
			{
				ptr<File> value = stmtSelectKeysToPush->ColumnBlob(2);
				size_t valueSize = value->GetSize();
				writer->WriteShortly(valueSize);
				if(valueSize)
					writer->Write(value->GetData(), valueSize);
			}

			{
				// get id of item
				long long itemId = stmtSelectKeysToPush->ColumnInt64(0);
				// change status of item to 'transient'
				ChangeKeyItemStatus(itemId, ItemStatuses::transient);
				// add item id to list
				transientIds.push_back(itemId);
			}

			break;

		case SQLITE_DONE:
			// no more keys to push
			done = true;
			break;

		default:
			THROW_SECONDARY("Error getting changes to push", db->Error());
		}
	}

	// write ending zero
	writer->WriteShortly(0);

	// commit transaction
	transaction.Commit();

	// set flag
	pushInProgress = true;

	END_TRY("Can't push repo");
}

void ClientRepo::Pull(StreamReader* reader)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	// read number of successfully commited keys
	size_t committedKeysCount = reader->ReadShortly();
	// process commited keys
	for(size_t i = 0; i < committedKeysCount; ++i)
	{
		// read key index
		size_t keyIndex = reader->ReadShortly();
		if(keyIndex >= transientIds.size())
			THROW("Invalid commited key index");

		long long transientId = transientIds[keyIndex];
		if(!transientId)
			THROW("Duplicate commited key index");
		transientIds[keyIndex] = 0;

		// get other key items
		KeyItems keyItems = GetKeyItemsByOneItemId(transientId);

		// committed key can't be conflicted

		// 'transient' becomes 'server'
		ChangeKeyItemStatus(keyItems.ids[ItemStatuses::transient], ItemStatuses::server);
		// 'postponed' (if presents) becomes 'client'
		if(keyItems.ids[ItemStatuses::postponed])
			ChangeKeyItemStatus(keyItems.ids[ItemStatuses::postponed], ItemStatuses::client);
	}

	// process conflicted keys
	for(size_t i = 0; i < transientIds.size(); ++i)
	{
		long long transientId = transientIds[i];
		if(!transientId)
			continue;

		// get other key items
		KeyItems keyItems = GetKeyItemsByOneItemId(transientId);

		// committed key, even failed, can't be conflicted before

		// if 'postponed' presents, it becomes 'conflictClient', and 'transient' disappears
		if(keyItems.ids[ItemStatuses::postponed])
		{
			ChangeKeyItemStatus(keyItems.ids[ItemStatuses::postponed], ItemStatuses::conflictClient);
			RemoveKeyItem(keyItems.ids[ItemStatuses::transient]);
		}
		// else 'transient' becomes 'conflictClient'
		else
			ChangeKeyItemStatus(keyItems.ids[ItemStatuses::transient], ItemStatuses::conflictClient);

		// 'server' (if presents) becomes 'conflictBase'
		if(keyItems.ids[ItemStatuses::server])
			ChangeKeyItemStatus(keyItems.ids[ItemStatuses::server], ItemStatuses::conflictBase);
	}

	// clear transient ids
	transientIds.clear();

	// read total number of keys in pull
	pullLag = reader->ReadShortlyBig();

	void* key = keyBufferFile->GetData();
	void* value = valueBufferFile->GetData();

	// loop for keys
	for(;;)
		try
		{
			// each key works in its own transaction
			Data::SqliteTransaction keyTransaction(db);

			// read key size
			size_t keySize = reader->ReadShortly();
			if(!keySize)
				break;
			// read key
			reader->Read(key, keySize);

			// read value size
			size_t valueSize = reader->ReadShortly();
			// read value
			if(valueSize)
				reader->Read(value, valueSize);

			// read revision
			long long revision = reader->ReadShortlyBig();

			ptr<File> keyFile = NEW(PartFile(keyBufferFile, key, keySize));
			ptr<File> valueFile = NEW(PartFile(valueBufferFile, value, valueSize));

			// get acquainted with this key
			KeyItems keyItems = GetKeyItems(keyFile);

			// new value always becomes 'server'
			if(keyItems.ids[ItemStatuses::server])
				ChangeKeyItemValue(keyItems.ids[ItemStatuses::server], valueFile);
			else
				AddKeyItem(keyFile, valueFile, ItemStatuses::server);

			// if there is 'client', it becomes 'conflictClient'
			if(keyItems.ids[ItemStatuses::client])
				ChangeKeyItemStatus(keyItems.ids[ItemStatuses::client], ItemStatuses::conflictClient);

			// set new global revision
			SetManifestValue(ManifestKeys::globalRevision, revision);

			// that key is ok
			keyTransaction.Commit();
		}
		catch(Exception* exception)
		{
			// skip errors while processing key
			// just rollback key transaction and stop processing
			// already processed keys will be committed
			MakePointer(exception);
			break;
		}

	// commit
	transaction.Commit();

	END_TRY("Can't pull repo");
}

void ClientRepo::Cleanup()
{
	BEGIN_TRY();

	// clean list of transient ids
	transientIds.clear();

	// change 'transient' items to 'client'
	{
		Data::SqliteQuery query(stmtMassChangeStatus);

		stmtMassChangeStatus->Bind(1, ItemStatuses::transient);
		stmtMassChangeStatus->Bind(2, ItemStatuses::client);

		if(stmtMassChangeStatus->Step() != SQLITE_DONE)
			THROW_SECONDARY("Can't change transient items to client", db->Error());
	}

	// change 'postponed' items to 'client' (possibly replacing former 'transient' ones)
	{
		Data::SqliteQuery query(stmtMassChangeStatus);

		stmtMassChangeStatus->Bind(1, ItemStatuses::postponed);
		stmtMassChangeStatus->Bind(2, ItemStatuses::client);

		if(stmtMassChangeStatus->Step() != SQLITE_DONE)
			THROW_SECONDARY("Can't change postponed items to client", db->Error());
	}

	END_TRY("Can't cleanup repo transients");
}

END_INANITY_OIL

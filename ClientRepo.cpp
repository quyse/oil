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
		/// server version
		server,
		/// client change based on 'server' in case of no conflict
		client,
		/// client change based on 'server' which is in process of committing to server
		transient,
		/// client change based on 'transient', waiting for results of commit of 'transient'
		postponed,

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

class ClientRepo::EventQueueTransaction
{
private:
	ClientRepo* repo;
	size_t eventsCount;

public:
	EventQueueTransaction(ClientRepo* repo)
	: repo(repo), eventsCount(repo->events.size()) {}

	~EventQueueTransaction()
	{
		repo->events.resize(eventsCount);
	}

	void Commit()
	{
		eventsCount = repo->events.size();
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
* client changed data
	server client
* changed data is commiting to server
	server transient
* client changed data, while previous change is still committing
	server transient postponed

Sync includes two substeps - push and pull.
Push tryes to commit client changes on server.
Pull gets server changes to client.

Algorithm of sync.

Request:
* Select subset of 'client' changes satisfying constraints.
* Transform these 'client's into 'transient's.
* Send keys and values of the subset in order.
Response:
* For each change:
	* 'transient' becomes 'server', old 'server' (if presents) disappears
	* 'postponed' (if presents) becomes 'client'
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

*/

ClientRepo::ClientRepo(const char* fileName)
: Repo(fileName), pushInProgress(false)
{
	BEGIN_TRY();

	// check format version
	CheckAppVersion(clientRepoAppVersion);

	// enable exclusive locking mode
	if(sqlite3_exec(*db, "PRAGMA locking_mode = EXCLUSIVE", 0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't enable exclusive locking mode on db", db->Error());
	// disable syncing
	if(sqlite3_exec(*db, "PRAGMA synchronous = OFF", 0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't disable db sync", db->Error());

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

	// create table chunks
	if(sqlite3_exec(*db,
		"CREATE TABLE IF NOT EXISTS chunks ("
		"prerev INTEGER PRIMARY KEY, "
		"postrev INTEGER NOT NULL)",
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create table chunks", db->Error());

	// create statements

	stmtManifestGet = db->CreateStatement("SELECT value FROM manifest WHERE key = ?1");
	stmtManifestSet = db->CreateStatement("INSERT OR REPLACE INTO manifest (key, value) VALUES (?1, ?2)");
	stmtGetKeyItems = db->CreateStatement("SELECT id, status FROM items WHERE key = ?1");
	stmtGetKeyItemsByOneItemId = db->CreateStatement("SELECT id, status FROM items WHERE key = (SELECT key FROM items WHERE id = ?1)");
	stmtGetKeyItemKey = db->CreateStatement("SELECT key FROM items WHERE id = ?1");
	stmtGetKeyItemValue = db->CreateStatement("SELECT value FROM items WHERE id = ?1");
	stmtGetKeyItemValueLength = db->CreateStatement("SELECT LENGTH(value) FROM items WHERE id = ?1");
	stmtAddKeyItem = db->CreateStatement("INSERT OR REPLACE INTO items (key, value, status) VALUES (?1, ?2, ?3)");
	stmtRemoveKeyItem = db->CreateStatement("DELETE FROM items WHERE id = ?1");
	stmtChangeKeyItemStatus = db->CreateStatement("UPDATE OR REPLACE items SET status = ?2 WHERE id = ?1");
	stmtChangeKeyItemValue = db->CreateStatement("UPDATE items SET value = ?2 WHERE id = ?1");
	{
		std::ostringstream ss;
		ss <<
			"SELECT a.id, a.key, a.value"
			" FROM items AS a LEFT JOIN items AS b ON a.key = b.key"
			" AND b.status = " << ItemStatuses::server
			<< " WHERE a.status = " << ItemStatuses::client
			<< " ORDER BY a.id LIMIT ?1";
		stmtSelectKeysToPush = db->CreateStatement(ss.str().c_str());
	}
	{
		std::ostringstream ss;
		ss <<
			"SELECT COUNT(a.id)"
			" FROM items AS a LEFT JOIN items AS b ON a.key = b.key"
			" AND b.status = " << ItemStatuses::server
			<< " WHERE a.status = " << ItemStatuses::client;
		stmtGetPushLag = db->CreateStatement(ss.str().c_str());
	}
	stmtMassChangeStatus = db->CreateStatement("UPDATE OR REPLACE items SET status = ?2 WHERE status = ?1");
	stmtEnumerateKeysBegin = db->CreateStatement("SELECT DISTINCT key FROM items WHERE key >= ?1 ORDER BY key");
	stmtEnumerateKeysBeginEnd = db->CreateStatement("SELECT DISTINCT key FROM items WHERE key >= ?1 AND key < ?2 ORDER BY key");
	stmtAddChunk = db->CreateStatement("INSERT INTO chunks (prerev, postrev) VALUES (?1, ?2)");
	stmtPreCutChunks = db->CreateStatement("SELECT MAX(postrev) FROM chunks WHERE prerev <= ?1");
	stmtCutChunks = db->CreateStatement("DELETE FROM chunks WHERE prerev <= ?1");
	stmtGetUpperRevision = db->CreateStatement("SELECT MIN(prerev) FROM chunks");

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

#ifdef _DEBUG
	{
		// check that this is a reasonable combination
		static const int reasonable[] =
		{
			0,
			(1 << ItemStatuses::client),
			(1 << ItemStatuses::transient),
			(1 << ItemStatuses::transient) | (1 << ItemStatuses::postponed)
		};
		static const int meaningful[] =
		{
			ItemStatuses::client,
			ItemStatuses::transient,
			ItemStatuses::postponed
		};
		int c = 0;
		for(size_t i = 0; i < sizeof(meaningful) / sizeof(meaningful[0]); ++i)
			if(keyItems.ids[meaningful[i]])
				c |= 1 << meaningful[i];
		size_t i;
		for(i = 0; i < sizeof(reasonable) / sizeof(reasonable[0]); ++i)
			if(reasonable[i] == c)
				break;
		if(i >= sizeof(reasonable) / sizeof(reasonable[0]))
			THROW("Key items combination doesn't seem reasonable");
	}
#endif


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

ptr<File> ClientRepo::GetKeyItemKey(long long itemId)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtGetKeyItemKey);

	stmtGetKeyItemKey->Bind(1, itemId);

	if(stmtGetKeyItemKey->Step() != SQLITE_ROW)
		THROW_SECONDARY("Error getting key item key", db->Error());

	return stmtGetKeyItemKey->ColumnBlob(0);
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

size_t ClientRepo::GetKeyItemValueLength(long long itemId)
{
	THROW_ASSERT(itemId > 0);

	Data::SqliteQuery query(stmtGetKeyItemValueLength);

	stmtGetKeyItemValue->Bind(1, itemId);

	if(stmtGetKeyItemValue->Step() != SQLITE_ROW)
		THROW_SECONDARY("Error getting key item value length", db->Error());

	return (size_t)stmtGetKeyItemValue->ColumnInt64(0);
}

ptr<File> ClientRepo::GetKeyValue(ptr<File> key)
{
	KeyItems keyItems = GetKeyItems(key);

	int order[] = {
		ItemStatuses::postponed,
		ItemStatuses::transient,
		ItemStatuses::client,
		ItemStatuses::server
	};

	for(int i = 0; i < sizeof(order) / sizeof(order[0]); ++i)
		if(keyItems.ids[order[i]])
			return GetKeyItemValue(keyItems.ids[order[i]]);

	return nullptr;
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

long long ClientRepo::GetGlobalRevision()
{
	// get client revision
	long long globalRevision = GetManifestValue(ManifestKeys::globalRevision, 0);

	// cut chunks with client revision (client revision may change)
	CutChunks(globalRevision);

	return globalRevision;
}

void ClientRepo::AddChunk(long long prePushRevision, long long postPushRevision)
{
	Data::SqliteQuery query(stmtAddChunk);

	stmtAddChunk->Bind(1, prePushRevision);
	stmtAddChunk->Bind(2, postPushRevision);

	if(stmtAddChunk->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error adding chunk", db->Error());
}

void ClientRepo::CutChunks(long long& globalRevision)
{
	// increase global revision if chunks allow
	for(;;)
	{
		Data::SqliteQuery query(stmtPreCutChunks);

		stmtPreCutChunks->Bind(1, globalRevision);

		if(stmtPreCutChunks->Step() != SQLITE_ROW)
			THROW_SECONDARY("Error getting pre-cut revision", db->Error());

		long long preCutRevision = stmtPreCutChunks->ColumnInt64(0);
		if(preCutRevision > globalRevision)
		{
			globalRevision = preCutRevision;
			SetManifestValue(ManifestKeys::globalRevision, globalRevision);
		}
		else
			break;
	}

	// remove chunks behind global revision
	Data::SqliteQuery query(stmtCutChunks);

	stmtCutChunks->Bind(1, globalRevision);

	if(stmtCutChunks->Step() != SQLITE_DONE)
		THROW_SECONDARY("Error cutting chunks", db->Error());
}

long long ClientRepo::GetUpperRevision()
{
	Data::SqliteQuery query(stmtGetUpperRevision);

	if(stmtGetUpperRevision->Step() != SQLITE_ROW)
		THROW_SECONDARY("Error getting upper revision", db->Error());

	// If there is no chunks the result is NULL,
	// but SQLite will automatically convert it into (int64) 0.
	// This is our intended behavior too.
	return stmtGetUpperRevision->ColumnInt64(0);
}

void ClientRepo::QueueEvent(ptr<File> key, ptr<File> value)
{
	events.push_back(std::pair<ptr<File>, ptr<File> >(key, value));
}

void ClientRepo::Change(ptr<File> key, ptr<File> value)
{
	BEGIN_TRY();

	// check key size
	if(key->GetSize() <= 0)
		THROW("Key length should be positive");

	// replace value with zero file
	ptr<File> valueToWrite = value ? value : emptyFile;

	Data::SqliteTransaction transaction(db);

	// get acquainted with this key
	KeyItems keyItems = GetKeyItems(key);

	// select status for the change
	int status;
	if(keyItems.ids[ItemStatuses::transient])
		status = ItemStatuses::postponed;
	else
		status = ItemStatuses::client;

	// if item is present, change it
	if(keyItems.ids[status])
		ChangeKeyItemValue(keyItems.ids[status], valueToWrite);
	// else add new item
	else
		AddKeyItem(key, valueToWrite, status);

	transaction.Commit();

	QueueEvent(key, value);

	END_TRY("Can't change repo value");
}

ptr<File> ClientRepo::GetValue(ptr<File> key)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	return GetKeyValue(key);

	END_TRY("Can't get repo value");
}

bool ClientRepo::HasValue(ptr<File> key)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	KeyItems keyItems = GetKeyItems(key);

	int order[] = {
		ItemStatuses::postponed,
		ItemStatuses::transient,
		ItemStatuses::client,
		ItemStatuses::server
	};

	for(int i = 0; i < sizeof(order) / sizeof(order[0]); ++i)
		if(keyItems.ids[order[i]] && GetKeyItemValueLength(keyItems.ids[order[i]]))
			return true;

	return false;

	END_TRY("Can't figure out if repo has value");
}

void ClientRepo::EnumerateKeys(ptr<File> prefix, KeyEnumerator* enumerator)
{
	BEGIN_TRY();

	size_t prefixSize = prefix->GetSize();

	// get upper bound for the range
	// copy prefix
	ptr<File> endRange = MemoryFile::CreateViaCopy(prefix->GetData(), prefixSize);
	// "increment" prefix
	unsigned char* endRangeData = (unsigned char*)endRange->GetData();
	size_t i;
	for(i = prefixSize; i > 0; --i)
		if(endRangeData[i - 1] < 0xff)
		{
			++endRangeData[i - 1];
			break;
		}
		else
			endRangeData[i - 1] = 0;
	// if we got to begin, all the data beginning from prefix belongs to the range
	if(!i)
		endRange = nullptr;

	Data::SqliteStatement* stmt = endRange ? stmtEnumerateKeysBeginEnd : stmtEnumerateKeysBegin;

	Data::SqliteQuery query(stmt);

	stmt->Bind(1, prefix);
	if(endRange)
		stmt->Bind(2, endRange);

	bool done = false;
	while(!done)
	{
		switch(stmt->Step())
		{
		case SQLITE_ROW:
			if(!enumerator->OnKey(stmt->ColumnBlob(0)))
				done = true;
			break;
		case SQLITE_DONE:
			done = true;
			break;
		default:
			THROW_SECONDARY("Error enumerating keys", db->Error());
		}
	}

	END_TRY("Can't enumerate repo keys");
}

void ClientRepo::EnumerateKeyValues(ptr<File> prefix, KeyValueEnumerator* enumerator)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	class Enumerator : public KeyEnumerator
	{
	private:
		ClientRepo* repo;
		KeyValueEnumerator* enumerator;

	public:
		Enumerator(ClientRepo* repo, KeyValueEnumerator* enumerator)
		: repo(repo), enumerator(enumerator) {}

		bool OnKey(ptr<File> key)
		{
			return enumerator->OnKeyValue(key, repo->GetKeyValue(key));
		}
	};

	EnumerateKeys(prefix, &Enumerator(this, enumerator));

	END_TRY("Can't enumerate repo key values");
}

void ClientRepo::ReadServerManifest(StreamReader* reader)
{
	BEGIN_TRY();

	// read and check protocol magic
	char protocolMagicValue[sizeof(protocolMagic)];
	reader->Read(protocolMagicValue, sizeof(protocolMagic));
	if(memcmp(protocolMagicValue, protocolMagic, sizeof(protocolMagic)) != 0)
		THROW("Invalid protocol magic");
	// read and check protocol version
	if(reader->ReadShortly() != protocolVersion)
		THROW("Invalid protocol version");
	// read constraints
	maxKeySize = reader->ReadShortly();
	maxValueSize = reader->ReadShortly();
	maxPushKeysCount = reader->ReadShortly();
	maxPushTotalSize = reader->ReadShortly();
	maxPullKeysCount = reader->ReadShortly();
	maxPullTotalSize = reader->ReadShortly();
	// check end
	reader->ReadEnd();

	END_TRY("Can't read server repo manifest");
}

void ClientRepo::Push(StreamWriter* writer)
{
	BEGIN_TRY();

	if(pushInProgress)
		THROW("Push already in progress");

	Data::SqliteTransaction transaction(db);

	// get client revision
	long long globalRevision = GetGlobalRevision();

	// write our revision
	writer->WriteShortlyBig(globalRevision);

	// write our upper revision
	{
		long long upperRevision = GetUpperRevision();
		writer->WriteShortlyBig(upperRevision);
	}

	// select keys to push
	THROW_ASSERT(transientIds.empty());

	Data::SqliteQuery query(stmtSelectKeysToPush);

	stmtSelectKeysToPush->Bind(1, maxPushKeysCount);

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

bool ClientRepo::Pull(StreamReader* reader)
{
	BEGIN_TRY();

	if(!pushInProgress)
		THROW("Push isn't in progress");

	Data::SqliteTransaction transaction(db);
	EventQueueTransaction eventQueueTransaction(this);

	// read total number of keys in pull
	pullLag = reader->ReadShortlyBig();

	// read pre-push revision
	long long prePushRevision = reader->ReadShortlyBig();
	// read post-push revision
	long long postPushRevision = reader->ReadShortlyBig();

	bool changedSomething = false;

	// process commited keys
	for(size_t i = 0; i < transientIds.size(); ++i)
	{
		long long transientId = transientIds[i];

		// get other key items
		KeyItems keyItems = GetKeyItemsByOneItemId(transientId);

		// 'transient' becomes 'server'
		ChangeKeyItemStatus(keyItems.ids[ItemStatuses::transient], ItemStatuses::server);
		// 'postponed' (if presents) becomes 'client'
		if(keyItems.ids[ItemStatuses::postponed])
			ChangeKeyItemStatus(keyItems.ids[ItemStatuses::postponed], ItemStatuses::client);

		changedSomething = true;
	}

	// add chunk if non-zero
	if(prePushRevision < postPushRevision)
		AddChunk(prePushRevision, postPushRevision);

	// set pushed count
	pushedKeysCount = (long long)transientIds.size();

	// clear transient ids
	transientIds.clear();

	// calculate push lag
	{
		Data::SqliteQuery query(stmtGetPushLag);

		switch(stmtGetPushLag->Step())
		{
		case SQLITE_ROW:
			pushLag = stmtGetPushLag->ColumnInt64(0);
			break;
		default:
			THROW("Can't determine push lag");
		}

		pushLag += pushedKeysCount;
	}

	// pull keys
	void* key = keyBufferFile->GetData();
	void* value = valueBufferFile->GetData();

	pulledKeysCount = 0;

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

			QueueEvent(MemoryFile::CreateViaCopy(keyFile), valueFile->GetSize() ? MemoryFile::CreateViaCopy(valueFile) : nullptr);

			// new value always becomes 'server'
			if(keyItems.ids[ItemStatuses::server])
				ChangeKeyItemValue(keyItems.ids[ItemStatuses::server], valueFile);
			else
				AddKeyItem(keyFile, valueFile, ItemStatuses::server);

			// set new global revision
			SetManifestValue(ManifestKeys::globalRevision, revision);

			// that key is ok
			keyTransaction.Commit();

			++pulledKeysCount;

			changedSomething = true;
		}
		catch(Exception* e)
		{
			// There was an error while pulling key.
			// This is sad; however, we support partial pulling.
			// Key transaction is rollbacked already,
			// but outer transaction could be committed.
			// Do cleanup after that and throw an exception.

			ptr<Exception> exception = e;

			transaction.Commit();
			eventQueueTransaction.Commit();

			// rethrow exception
			THROW_SECONDARY("Error pulling keys", exception);
		}

	// read and save new client revision
	SetManifestValue(ManifestKeys::globalRevision, reader->ReadShortlyBig());

	// check end
	reader->ReadEnd();

	// commit
	transaction.Commit();
	eventQueueTransaction.Commit();

	pushInProgress = false;

	return changedSomething;

	END_TRY("Can't pull repo");
}

void ClientRepo::Cleanup()
{
	BEGIN_TRY();

	pushInProgress = false;

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

void ClientRepo::WriteWatchRequest(StreamWriter* writer)
{
	BEGIN_TRY();

	Data::SqliteTransaction transaction(db);

	long long globalRevision = GetGlobalRevision();

	writer->WriteShortlyBig(globalRevision);

	END_TRY("Can't do repo watch");
}

bool ClientRepo::ReadWatchResponse(StreamReader* reader)
{
	BEGIN_TRY();

	long long serverRevision = reader->ReadShortlyBig();
	reader->ReadEnd();

	Data::SqliteTransaction transaction(db);

	// get client revision
	long long globalRevision = GetGlobalRevision();

	// if we have what to pull, sync is needed
	if(globalRevision < serverRevision)
		return true;

	// if we have what to push, sync is needed
	Data::SqliteQuery query(stmtSelectKeysToPush);
	stmtSelectKeysToPush->Bind(1, 1); // maximum number of keys to push
	switch(stmtSelectKeysToPush->Step())
	{
	case SQLITE_ROW:
		return true;
	case SQLITE_DONE:
		return false;
	default:
		THROW_SECONDARY("Can't select keys to push", db->Error());
	}

	END_TRY("Can't get if repo sync needed");
}

void ClientRepo::ProcessEvents(EventHandler* eventHandler)
{
	if(eventHandler)
		for(size_t i = 0; i < events.size(); ++i)
		{
			eventHandler->OnEvent(events[i].first, events[i].second);
			events[i].first = nullptr;
			events[i].second = nullptr;
		}
	events.clear();
}

long long ClientRepo::GetPushLag() const
{
	return pushLag;
}

long long ClientRepo::GetPushedKeysCount() const
{
	return pushedKeysCount;
}

long long ClientRepo::GetPullLag() const
{
	return pullLag;
}

long long ClientRepo::GetPulledKeysCount() const
{
	return pulledKeysCount;
}

END_INANITY_OIL

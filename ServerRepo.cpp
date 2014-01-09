#include "ServerRepo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/PartFile.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

/*

Push:

-> client revision (shortly big)
-> client upper revision (shortly big)
->{
    key size (shortly)
    key data[key size]
    value size (shortly)
    value data[value size]
  }
  0 (shortly)

<- total number of changed unique keys since client revision (shortly big)
<- pre-push revision (shortly big)
<- 1-based index of write[number of successful writes] (shortly)
<- 0 (shortly)
<- post-push revision (shortly big)
<-{
    key size (shortly)
    key data[key size]
    value size (shortly)
    value data[value size]
    revision (shortly big)
  }
  0 (shortly)
<- new client revision (shortly big)

*/

ServerRepo::ServerRepo(const char* fileName)
: Repo(fileName)
{
	BEGIN_TRY();

	// create table revs
	if(sqlite3_exec(*db,
		"CREATE TABLE IF NOT EXISTS revs ("
		"rev INTEGER PRIMARY KEY AUTOINCREMENT, "
		"key BLOB NOT NULL, "
		"value BLOB NOT NULL)",
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create table revs", db->Error());
	// create index revs_key_rev
	if(sqlite3_exec(*db,
		"CREATE INDEX IF NOT EXISTS revs_key_rev ON revs (key, rev)",
		0, 0, 0) != SQLITE_OK)
		THROW_SECONDARY("Can't create index revs_key_rev", db->Error());

	// create statements
	stmtGetMaxRevision = db->CreateStatement("SELECT MAX(rev) FROM revs");
	stmtCheckConflict = db->CreateStatement("SELECT COUNT(*) FROM revs WHERE key = ?1 AND rev > ?2");
	stmtWrite = db->CreateStatement("INSERT INTO revs (key, value) VALUES (?1, ?2)");
#define SUBQUERY "SELECT key, MAX(rev) AS maxrev FROM revs GROUP BY key"
	stmtPull = db->CreateStatement(
		"SELECT rev, key, value FROM revs NATURAL JOIN ("
		SUBQUERY
		") WHERE rev > ?1 AND rev <= ?2 AND rev = maxrev ORDER BY rev LIMIT ?3");
	stmtGetWeakRevision = db->CreateStatement(
		"SELECT rev FROM revs NATURAL JOIN ("
		SUBQUERY
		") WHERE rev > ?1 AND rev <= ?2 AND rev = maxrev ORDER BY rev LIMIT 1");
#undef SUBQUERY
	stmtPullTotalSize = db->CreateStatement(
		"SELECT COUNT(DISTINCT key) FROM revs WHERE rev > ?1");

	END_TRY("Can't create server repo");
}

long long ServerRepo::GetMaxRevision()
{
	Data::SqliteQuery query(stmtGetMaxRevision);

	if(stmtGetMaxRevision->Step() != SQLITE_ROW)
		THROW_SECONDARY("Can't get max revision", db->Error());

	return stmtGetMaxRevision->ColumnInt64(0);
}

void ServerRepo::WriteManifest(StreamWriter* writer)
{
	BEGIN_TRY();

	// write protocol magic
	writer->Write(protocolMagic, sizeof(protocolMagic));
	// write protocol version
	writer->WriteShortly(1);
	// write constraints
	writer->WriteShortly(maxKeySize);
	writer->WriteShortly(maxValueSize);
	writer->WriteShortly(maxPushKeysCount);
	writer->WriteShortly(maxPushTotalSize);
	writer->WriteShortly(maxPullKeysCount);
	writer->WriteShortly(maxPullTotalSize);

	END_TRY("Can't write server repo manifest");
}

void ServerRepo::Sync(StreamReader* reader, StreamWriter* writer)
{
	BEGIN_TRY();

	// push & pull should be in a one transaction
	Data::SqliteTransaction transaction(db);

	// get initial revision
	long long prePushRevision = GetMaxRevision();

	// read client revision
	long long clientRevision = reader->ReadShortlyBig();
	// read client upper revision
	long long clientUpperRevision = reader->ReadShortlyBig();
	// correct upper revision
	if(clientUpperRevision == 0 || clientUpperRevision > prePushRevision)
		clientUpperRevision = prePushRevision;

	// determine total size of the pull
	{
		Data::SqliteQuery queryPullTotalSize(stmtPullTotalSize);

		stmtPullTotalSize->Bind(1, clientRevision);
		if(stmtPullTotalSize->Step() != SQLITE_ROW)
			THROW("Can't determine total pull size");

		// output total size of pull
		long long pullTotalSize = stmtPullTotalSize->ColumnInt64(0);
		writer->WriteShortlyBig(pullTotalSize);
	}

	//*** push

	void* key = keyBufferFile->GetData();
	void* value = valueBufferFile->GetData();

	size_t totalPushSize = 0;

	// output pre-push revision
	writer->WriteShortlyBig(prePushRevision);

	// loop for push keys
	for(size_t i = 0; ; ++i)
	{
		// read key size
		size_t keySize = reader->ReadShortly();
		// if it's 0, it's signal of end
		if(!keySize)
			break;

		// check number of keys
		if(i >= (size_t)maxPushKeysCount)
			THROW("Too many keys");

		// check key size
		if(keySize > maxKeySize)
			THROW("Too big key");

		// read key value
		reader->Read(key, keySize);

		// read value
		size_t valueSize = reader->ReadShortly();
		if(valueSize > maxValueSize)
			THROW("Too big value");
		if(valueSize)
			reader->Read(value, valueSize);

		// check total push size
		totalPushSize += valueSize;
		if(totalPushSize > maxPushTotalSize)
			THROW("Too big push total size");

		// check for conflict
		Data::SqliteQuery queryCheckConflict(stmtCheckConflict);
		ptr<File> keyFile = NEW(PartFile(keyBufferFile, key, keySize));
		stmtCheckConflict->Bind(1, keyFile);
		stmtCheckConflict->Bind(2, clientRevision);
		if(stmtCheckConflict->Step() != SQLITE_ROW)
			THROW_SECONDARY("Can't check conflict", db->Error());
		if(stmtCheckConflict->ColumnInt(0) > 0)
			// conflict, skip this
			continue;

		// do write
		Data::SqliteQuery queryWrite(stmtWrite);
		stmtWrite->Bind(1, keyFile);
		stmtWrite->Bind(2, NEW(PartFile(valueBufferFile, value, valueSize)));
		if(stmtWrite->Step() != SQLITE_DONE)
			THROW_SECONDARY("Can't do write", db->Error());

		// output write
		writer->WriteShortly(i + 1);
	}

	// ensure request is over
	reader->ReadEnd();

	// output zero
	writer->WriteShortly(0);

	// retrieve and output post-push revision
	long long postPushRevision = GetMaxRevision();
	writer->WriteShortlyBig(postPushRevision);

	//*** pull
	Data::SqliteQuery queryPull(stmtPull);

	stmtPull->Bind(1, clientRevision);
	stmtPull->Bind(2, clientUpperRevision);
	stmtPull->Bind(3, maxPullKeysCount);

	long long lastKnownClientRevision = clientRevision;

	size_t totalPullSize = 0;
	bool done = false;
	while(!done)
	{
		switch(stmtPull->Step())
		{
		case SQLITE_ROW:
			// check total pull size
			totalPullSize += stmtPull->ColumnBlobSize(2);
			if(totalPullSize > maxPullTotalSize)
			{
				// stop pull, that's enough
				done = true;
				break;
			}

			// output key
			{
				ptr<File> key = stmtPull->ColumnBlob(1);
				size_t keySize = key->GetSize();
				writer->WriteShortly(keySize);
				writer->Write(key->GetData(), keySize);
			}
			// output value
			{
				ptr<File> value = stmtPull->ColumnBlob(2);
				size_t valueSize = value->GetSize();
				writer->WriteShortly(valueSize);
				writer->Write(value->GetData(), valueSize);
			}
			{
				// output revision
				long long revision = stmtPull->ColumnInt64(0);
				writer->WriteShortlyBig(revision);

				// remember last revision
				lastKnownClientRevision = revision;
			}

			break;

		case SQLITE_DONE:
			// end, no more keys
			done = true;
			break;

		default:
			// some error
			THROW_SECONDARY("Error pulling changes", db->Error());
		}
	}

	// write final zero
	writer->WriteShortly(0);

	// write new client revision
	{
		Data::SqliteQuery query(stmtGetWeakRevision);

		stmtGetWeakRevision->Bind(1, lastKnownClientRevision);
		stmtGetWeakRevision->Bind(2, clientUpperRevision);

		long long newClientRevision;
		switch(stmtGetWeakRevision->Step())
		{
		case SQLITE_ROW:
			newClientRevision = stmtGetWeakRevision->ColumnInt64(0) - 1;
			break;
		case SQLITE_DONE:
			newClientRevision = clientUpperRevision;
			break;
		default:
			THROW_SECONDARY("Error getting new client revision", db->Error());
		}

		writer->WriteShortlyBig(newClientRevision);
	}

	// commit transaction
	transaction.Commit();

	END_TRY("Can't sync server repo");
}

END_INANITY_OIL

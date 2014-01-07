#include "ServerRepo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/PartFile.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/Exception.hpp"
#include <sstream>

BEGIN_INANITY_OIL

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
	stmtCheckConflict = db->CreateStatement("SELECT COUNT(*) FROM revs WHERE key = ?1 AND rev > ?2");
	stmtWrite = db->CreateStatement("INSERT INTO revs (key, value) VALUES (?1, ?2)");
	{
		std::ostringstream ss;
		ss <<
			"SELECT rev, key, value FROM revs NATURAL JOIN ("
				"SELECT key, MAX(rev) AS maxrev FROM revs WHERE rev > ?1 GROUP BY key"
				") WHERE rev = maxrev ORDER BY rev LIMIT "
			<< maxPullKeysCount;
		stmtPull = db->CreateStatement(ss.str().c_str());
	}
	stmtPullTotalSize = db->CreateStatement(
		"SELECT COUNT(DISTINCT key) FROM revs WHERE rev > ?1");

	END_TRY("Can't create server repo");
}

void ServerRepo::Sync(StreamReader* reader, StreamWriter* writer)
{
	BEGIN_TRY();

	// push & pull should be in a one transaction
	Data::SqliteTransaction transaction(db);

	// read client revision
	long long clientRevision = reader->ReadShortlyBig();

	// do actual push
	DoPush(clientRevision, reader);

	// output number of successful writes
	writer->WriteShortly(written.size());
	// output successful writes
	for(size_t i = 0; i < written.size(); ++i)
		writer->WriteShortly(written[i]);

	// now do pull
	DoPull(clientRevision, writer);

	// commit transaction
	transaction.Commit();

	// output final zero (which is signal that's all right)
	writer->WriteShortly(0);

	END_TRY("Can't sync server repo");
}

void ServerRepo::DoPush(long long clientRevision, StreamReader* reader)
{
	void* key = keyBufferFile->GetData();
	void* value = valueBufferFile->GetData();

	written.clear();

	size_t totalPushSize = 0;

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

		// remember write
		written.push_back(i);
	}

	// ensure request is over
	reader->ReadEnd();
}

void ServerRepo::DoPull(long long clientRevision, StreamWriter* writer)
{
	// determine total size of the pull
	{
		Data::SqliteQuery queryPullTotalSize(stmtPullTotalSize);

		stmtPullTotalSize->Bind(1, clientRevision);
		if(stmtPullTotalSize->Step() != SQLITE_ROW)
			THROW("Can't determine total pull size");

		// output total size of pull
		writer->WriteShortlyBig(stmtPullTotalSize->ColumnInt64(0));
	}

	Data::SqliteQuery queryPull(stmtPull);

	stmtPull->Bind(1, clientRevision);

	size_t totalPullSize = 0;
	bool done = false;
	while(!done)
	{
		switch(stmtPull->Step())
		{
		case SQLITE_ROW:
			// check total pull size
			totalPullSize += stmtPull->ColumnBlobSize(1);
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
			// output revision
			writer->WriteShortlyBig(stmtPull->ColumnInt64(0));

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
}

END_INANITY_OIL

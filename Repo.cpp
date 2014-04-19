#include "Repo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"
#include <sstream>

BEGIN_INANITY_OIL

const char Repo::protocolMagic[14] = { 'I', 'N', 'A', 'N', 'I', 'T', 'Y', 'O', 'I', 'L', 'R', 'E', 'P', 'O' };
const int Repo::protocolVersion = 1;

const int Repo::serverRepoAppVersion = 0x424C494F; // "OILB" in little-endian
const int Repo::clientRepoAppVersion = 0x314C494F; // "OIL1" in little-endian

const size_t Repo::defaultMaxKeySize = 128;
const size_t Repo::defaultMaxValueSize = 1024 * 1024;
const int Repo::defaultMaxPushKeysCount = 1024;
const size_t Repo::defaultMaxPushTotalSize = 1024 * 1024 * 2;
const int Repo::defaultMaxPullKeysCount = 1024;
const size_t Repo::defaultMaxPullTotalSize = 1024 * 1024 * 2;

const char* Repo::fileNameMemory = ":memory:";
const char* Repo::fileNameTemp = "";

Repo::Repo(const char* fileName) :
	maxKeySize(defaultMaxKeySize),
	maxValueSize(defaultMaxValueSize),
	maxPushKeysCount(defaultMaxPushKeysCount),
	maxPushTotalSize(defaultMaxPushTotalSize),
	maxPullKeysCount(defaultMaxPullKeysCount),
	maxPullTotalSize(defaultMaxPullTotalSize)
{
	BEGIN_TRY();

	db = Data::SqliteDb::Open(fileName);

	keyBufferFile = NEW(MemoryFile(maxKeySize));
	valueBufferFile = NEW(MemoryFile(maxValueSize));

	END_TRY("Can't create repo");
}

void Repo::CheckAppVersion(int appVersion)
{
	ptr<Data::SqliteStatement> stmt = db->CreateStatement("PRAGMA application_id");
	if(stmt->Step() != SQLITE_ROW)
		THROW_SECONDARY("Error getting application_id", db->Error());

	int existingAppVersion = stmt->ColumnInt(0);
	if(existingAppVersion == 0)
	{
		// app version hasn't been set yet; set it
		std::ostringstream ss;
		ss << "PRAGMA application_id = " << appVersion;
		stmt = db->CreateStatement(ss.str().c_str());
		if(stmt->Step() != SQLITE_DONE)
			THROW_SECONDARY("Error setting application_id", db->Error());
	}
	else if(existingAppVersion == appVersion)
	{
		// all ok
	}
	else
		THROW("Wrong repo format");
}

END_INANITY_OIL

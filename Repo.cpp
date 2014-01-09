#include "Repo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

const size_t Repo::defaultMaxKeySize = 128;
const size_t Repo::defaultMaxValueSize = 1024 * 1024 * 16;
const int Repo::defaultMaxPushKeysCount = 128;
const size_t Repo::defaultMaxPushTotalSize = 1024 * 1024 * 32;
const int Repo::defaultMaxPullKeysCount = 256;
const size_t Repo::defaultMaxPullTotalSize = 1024 * 1024 * 32;

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

END_INANITY_OIL

#include "Repo.hpp"
#include "../inanity/data/sqlite.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

const size_t Repo::maxKeySize = 128;
const size_t Repo::maxValueSize = 1024 * 1024 * 16;
const int Repo::maxPushKeysCount = 128;
const size_t Repo::maxPushTotalSize = 1024 * 1024 * 32;
const int Repo::maxPullKeysCount = 256;
const size_t Repo::maxPullTotalSize = 1024 * 1024 * 32;

Repo::Repo(const char* fileName)
{
	BEGIN_TRY();

	db = Data::SqliteDb::Open(fileName);

	END_TRY("Can't create repo");
}

END_INANITY_OIL

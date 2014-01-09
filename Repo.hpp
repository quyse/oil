#ifndef ___INANITY_OIL_REPO_HPP___
#define ___INANITY_OIL_REPO_HPP___

#include "oil.hpp"
#include "../inanity/data/data.hpp"

BEGIN_INANITY

class MemoryFile;

END_INANITY

BEGIN_INANITY_DATA

class SqliteDb;
class SqliteStatement;

END_INANITY_DATA

BEGIN_INANITY_OIL

/// Base class for client and server repos.
class Repo : public Object
{
public:
	//*** Default security constraints.
	static const size_t defaultMaxKeySize;
	static const size_t defaultMaxValueSize;
	/// Maximum number key-value pairs in a push.
	static const int defaultMaxPushKeysCount;
	/// Maximum total size of values in a push.
	static const size_t defaultMaxPushTotalSize;
	/// Maximum number of key-value pairs in a pull response.
	static const int defaultMaxPullKeysCount;
	/// Maximum total size of values in a pull response.
	static const size_t defaultMaxPullTotalSize;

protected:
	/// SQLite database.
	ptr<Data::SqliteDb> db;

	//*** Temporaries.
	/// Key buffer.
	ptr<MemoryFile> keyBufferFile;
	/// Value buffer.
	ptr<MemoryFile> valueBufferFile;

public: // for simplicity
	//*** Security constraints.
	size_t maxKeySize;
	size_t maxValueSize;
	int maxPushKeysCount;
	size_t maxPushTotalSize;
	int maxPullKeysCount;
	size_t maxPullTotalSize;

public:
	Repo(const char* fileName);
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_REPO_HPP___
#define ___INANITY_OIL_REPO_HPP___

#include "oil.hpp"
#include "../inanity/data/data.hpp"

BEGIN_INANITY_DATA

class SqliteDb;
class SqliteStatement;

END_INANITY_DATA

BEGIN_INANITY_OIL

/// Base class for client and server repos.
class Repo : public Object
{
public:
	//*** Security constraints.
	static const size_t maxKeySize;
	static const size_t maxValueSize;
	/// Maximum number key-value pairs in a push.
	static const int maxPushKeysCount;
	/// Maximum total size of values in a push.
	static const size_t maxPushTotalSize;
	/// Maximum number of key-value pairs in a pull response.
	static const int maxPullKeysCount;
	/// Maximum total size of values in a pull response.
	static const size_t maxPullTotalSize;

protected:
	/// SQLite database.
	ptr<Data::SqliteDb> db;

public:
	Repo(const char* fileName);
};

END_INANITY_OIL

#endif

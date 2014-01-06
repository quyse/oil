#ifndef ___INANITY_OIL_SERVER_REPO_HPP___
#define ___INANITY_OIL_SERVER_REPO_HPP___

#include "Repo.hpp"
#include <vector>

BEGIN_INANITY

class StreamReader;
class StreamWriter;

END_INANITY

BEGIN_INANITY_OIL

class ServerRepo : public Repo
{
private:
	//*** SQLite statements.
	ptr<Data::SqliteStatement> stmtCheckConflict;
	ptr<Data::SqliteStatement> stmtWrite;
	ptr<Data::SqliteStatement> stmtPull;
	ptr<Data::SqliteStatement> stmtPullTotalSize;

	//*** Temporaries.
	/// Indexes of written values.
	std::vector<size_t> written;

	void DoPush(long long clientRevision, StreamReader* reader);
	void DoPull(long long clientRevision, StreamWriter* writer);

public:
	ServerRepo(const char* fileName);

	/// Sync with client.
	void Sync(StreamReader* reader, StreamWriter* writer);
};

END_INANITY_OIL

#endif

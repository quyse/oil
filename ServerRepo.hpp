#ifndef ___INANITY_OIL_SERVER_REPO_HPP___
#define ___INANITY_OIL_SERVER_REPO_HPP___

#include "Repo.hpp"

BEGIN_INANITY

class StreamReader;
class StreamWriter;

END_INANITY

BEGIN_INANITY_OIL

class ServerRepo : public Repo
{
private:
	//*** SQLite statements.
	ptr<Data::SqliteStatement> stmtGetMaxRevision;
	ptr<Data::SqliteStatement> stmtCheckConflict;
	ptr<Data::SqliteStatement> stmtWrite;
	ptr<Data::SqliteStatement> stmtPull;
	ptr<Data::SqliteStatement> stmtGetWeakRevision;
	ptr<Data::SqliteStatement> stmtPullTotalSize;

	long long GetMaxRevision();

public:
	ServerRepo(const char* fileName);

	/// Write manifest.
	void WriteManifest(StreamWriter* writer);
	/// Sync with client.
	void Sync(StreamReader* reader, StreamWriter* writer);
};

END_INANITY_OIL

#endif

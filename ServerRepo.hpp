#ifndef ___INANITY_OIL_SERVER_REPO_HPP___
#define ___INANITY_OIL_SERVER_REPO_HPP___

#include "Repo.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY

class StreamReader;
class StreamWriter;

END_INANITY

BEGIN_INANITY_OIL

class ServerRepo : public Repo
{
private:
	//*** SQLite statements.
	ptr<Data::SqliteStatement>
		stmtGetMaxRevision,
		stmtClearLatest,
		stmtWrite,
		stmtPull,
		stmtGetWeakRevision,
		stmtPullTotalSize,
		stmtGetUserId,
		stmtAddUser;

public:
	ServerRepo(const char* fileName);

	/// Get global revision.
	long long GetMaxRevision();
	/// Get user id.
	/** If there is no such login in user table, assign new id. */
	long long GetUserId(const String& name);
	/// Write manifest.
	void WriteManifest(StreamWriter* writer);
	/// Sync with client.
	/** Returns true if something was pushed. */
	bool Sync(StreamReader* reader, StreamWriter* writer, const String& userName, bool writeAccess);
	/// Process watch request.
	/** Returns if watch was responded. */
	bool Watch(StreamReader* reader, StreamWriter* writer);
	/// Create watch response.
	/** Should be good for every client. */
	void RespondWatch(StreamWriter* writer);
};

END_INANITY_OIL

#endif

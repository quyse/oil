#ifndef ___INANITY_OIL_CLIENT_REPO_HPP___
#define ___INANITY_OIL_CLIENT_REPO_HPP___

#include "Repo.hpp"
#include <vector>

BEGIN_INANITY

class File;
class StreamReader;
class StreamWriter;

END_INANITY

BEGIN_INANITY_OIL

class ClientRepo : public Repo
{
public:
	/// Interface to receive notifications about values.
	class EventHandler
	{
	public:
		virtual void OnEvent(ptr<File> key, ptr<File> value) = 0;
	};

	class KeysEnumerator
	{
	public:
		virtual void OnKey(ptr<File> key) = 0;
	};

private:
	struct ItemStatuses;
	struct ManifestKeys;
	struct KeyItems;
	class EventQueueTransaction;

	//*** SQLite statements.
	ptr<Data::SqliteStatement>
		stmtManifestGet,
		stmtManifestSet,
		stmtGetKeyItems,
		stmtGetKeyItemsByOneItemId,
		stmtGetKeyItemKey,
		stmtGetKeyItemValue,
		stmtGetKeyItemValueLength,
		stmtAddKeyItem,
		stmtRemoveKeyItem,
		stmtChangeKeyItemStatus,
		stmtChangeKeyItemValue,
		stmtSelectKeysToPush,
		stmtMassChangeStatus,
		stmtEnumerateKeysBegin,
		stmtEnumerateKeysBeginEnd,
		stmtAddChunk,
		stmtPreCutChunks,
		stmtCutChunks,
		stmtGetUpperRevision;

	/// Helper empty file.
	ptr<File> emptyFile;
	/// Is push in progress.
	bool pushInProgress;
	/// Ids of transient keys.
	std::vector<long long> transientIds;
	/// Number of keys to pull more (estimated).
	long long pullLag;

	/// Deferred events queue.
	std::vector<std::pair<ptr<File>, ptr<File> > > events;

	//*** Internal help methods.
	/// Check item status for validity.
	void CheckItemStatus(int itemStatus);
	/// Helper method for getting key items.
	KeyItems FillKeyItems(Data::SqliteStatement* stmt);
	/// Get basic info about key.
	KeyItems GetKeyItems(ptr<File> key);
	/// Get basic info about key using id of one of key items.
	KeyItems GetKeyItemsByOneItemId(long long itemId);
	/// Get key of key item.
	ptr<File> GetKeyItemKey(long long itemId);
	/// Get value of key item.
	ptr<File> GetKeyItemValue(long long itemId);
	/// Get value length of key item.
	size_t GetKeyItemValueLength(long long itemId);
	/// Add new key item (replacing old one if exists).
	void AddKeyItem(ptr<File> key, ptr<File> value, int status);
	/// Remove key item.
	void RemoveKeyItem(long long itemId);
	/// Change key item status (replacing old item if exists).
	void ChangeKeyItemStatus(long long itemId, int newItemStatus);
	/// Change key item value.
	void ChangeKeyItemValue(long long itemId, ptr<File> newValue);

	//*** Manifest methods.
	long long GetManifestValue(int key, long long defaultValue);
	void SetManifestValue(int key, long long value);
	long long GetGlobalRevision();

	//*** Chunks methods.
	void AddChunk(long long prePushRevision, long long postPushRevision);
	/// Removes chunks which behind global revision.
	/// Assigns global revision to post-push revision of last removed chunk.
	void CutChunks(long long& globalRevision);
	/// Get upper revision for pulling.
	/** 0 if no upper bound for pulling. */
	long long GetUpperRevision();

	/// Add deferred event.
	void QueueEvent(ptr<File> key, ptr<File> value);

public:
	ClientRepo(const char* fileName);

	ptr<Data::SqliteDb> GetDb() const;

	/// Add client change.
	/** If the key is conflicted, just change the client value.
	To remove key-value pair specify value = nullptr. */
	void Change(ptr<File> key, ptr<File> value);

	/// Get client value.
	ptr<File> GetValue(ptr<File> key);
	/// Check if client value is non-null.
	bool HasValue(ptr<File> key);
	/// Get keys with specified prefix.
	/** Only keys with non-null values returned. */
	void EnumerateKeys(ptr<File> prefix, KeysEnumerator* enumerator);

	void ReadServerManifest(StreamReader* reader);
	void Push(StreamWriter* writer);
	/// Process sync response.
	/** Returns if something changed. */
	bool Pull(StreamReader* reader);
	void Cleanup();
	void WriteWatchRequest(StreamWriter* writer);
	/// Read watch response.
	/** Returns if we need to sync. */
	bool ReadWatchResponse(StreamReader* reader);

	/// Send deferred events to event handler.
	void ProcessEvents(EventHandler* eventHandler);
};

END_INANITY_OIL

#endif

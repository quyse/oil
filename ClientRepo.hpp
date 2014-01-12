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
	/// Status of key.
	enum KeyStatus
	{
		/// client and server values are the same
		keyStatusSync,
		/// client value has changed after server value
		keyStatusAhead,
		/// client and server values diverged
		keyStatusConflicted
	};

	enum EventFlags
	{
		/// Value has been changed.
		/** Happens with any key status. */
		eventChanged = 1,

		//*** Status events.
		/// Value has become in sync.
		eventSync = 2,
		/// Value has become ahead of server.
		eventAhead = 4,
		/// Value has become conflicted.
		eventConflicted = 8,

		/// Conflicted server value has become identified.
		/** Could arrive with eventConflicted or separate. */
		eventBranch = 16
	};

	/// Interface to receive notifications about values.
	class EventHandler : public Object
	{
	public:
		virtual void OnEvent(ptr<File> key, int eventFlags);
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
		stmtAddKeyItem,
		stmtRemoveKeyItem,
		stmtChangeKeyItemStatus,
		stmtChangeKeyItemStatusAndRev,
		stmtChangeKeyItemValue,
		stmtChangeKeyItemValueAndRev,
		stmtSelectKeysToPush,
		stmtMassChangeStatus,
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

	ptr<EventHandler> eventHandler;
	/// Deferred events queue.
	std::vector<std::pair<ptr<File>, int> > events;

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
	/// Add new key item (replacing old one if exists).
	void AddKeyItem(ptr<File> key, ptr<File> value, int status, long long revision);
	/// Remove key item.
	void RemoveKeyItem(long long itemId);
	/// Change key item status (replacing old item if exists).
	void ChangeKeyItemStatus(long long itemId, int newItemStatus);
	void ChangeKeyItemStatusAndRev(long long itemId, int newItemStatus, long long newRevision);
	/// Change key item value.
	void ChangeKeyItemValue(long long itemId, ptr<File> newValue);
	void ChangeKeyItemValueAndRev(long long itemId, ptr<File> newValue, long long newRevision);

	//*** Manifest methods.
	long long GetManifestValue(int key, long long defaultValue);
	void SetManifestValue(int key, long long value);

	//*** Chunks methods.
	void AddChunk(long long prePushRevision, long long postPushRevision);
	/// Removes chunks which behind global revision.
	/// Assigns global revision to post-push revision of last removed chunk.
	void CutChunks(long long& globalRevision);
	/// Get upper revision for pulling.
	/** 0 if no upper bound for pulling. */
	long long GetUpperRevision();

	/// Add deferred event.
	void QueueEvent(ptr<File> key, int eventFlags);

public:
	ClientRepo(const char* fileName);

	static ptr<ClientRepo> CreateInMemory();

	ptr<Data::SqliteDb> GetDb() const;

	/// Add client change.
	/** If the key is conflicted, just change the client value.
	To remove key-value pair specify value = nullptr. */
	void Change(ptr<File> key, ptr<File> value);
	/// Resolve conflicting change.
	/** In case of no conflict throws.
	Just set that client change is not conflicted anymore. */
	void Resolve(ptr<File> key);

	/// Get key status.
	KeyStatus GetKeyStatus(ptr<File> key);
	/// Get client value.
	ptr<File> GetValue(ptr<File> key);
	/// Get is value in conflict.
	bool IsConflicted(ptr<File> key);
	/// Get conflicting (server) value.
	/** In case of no conflict throws.
	In case conflict server value isn't known yet, returns 0. */
	ptr<File> GetConflictServerValue(ptr<File> key);
	/// Get conflict base value.
	/** In case of no conflict throws.
	In case there is no base (because conflict of new values), returns 0. */
	ptr<File> GetConflictBaseValue(ptr<File> key);

	void ReadServerManifest(StreamReader* reader);
	void Push(StreamWriter* writer);
	void Pull(StreamReader* reader);
	void Cleanup();

	/// Set event handler.
	void SetEventHandler(ptr<EventHandler> eventHandler);
	/// Send deferred events to event handler.
	void ProcessEvents();
};

END_INANITY_OIL

#endif

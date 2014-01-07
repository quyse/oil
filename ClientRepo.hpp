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
		keyStatusSynced,
		/// client value has changed after server value
		keyStatusChanged,
		/// client and server values diverged
		keyStatusConflicted
	};

	/// Interface to receive notifications about values.
	class ChangeHandler : public Object
	{
	public:
		virtual void ValueChanged(ptr<File> key, ptr<File> clientValue, ptr<File> serverValue) = 0;
		virtual void StatusChanged(ptr<File> key, KeyStatus status) = 0;
	};

private:
	struct ItemStatuses;
	struct ManifestKeys;
	struct KeyItems;

	//*** SQLite statements.
	ptr<Data::SqliteStatement> stmtManifestGet;
	ptr<Data::SqliteStatement> stmtManifestSet;
	ptr<Data::SqliteStatement> stmtGetKeyItems;
	ptr<Data::SqliteStatement> stmtGetKeyItemsByOneItemId;
	ptr<Data::SqliteStatement> stmtGetKeyItemValue;
	ptr<Data::SqliteStatement> stmtAddKeyItem;
	ptr<Data::SqliteStatement> stmtRemoveKeyItem;
	ptr<Data::SqliteStatement> stmtChangeKeyItemStatus;
	ptr<Data::SqliteStatement> stmtChangeKeyItemValue;
	ptr<Data::SqliteStatement> stmtSelectKeysToPush;
	ptr<Data::SqliteStatement> stmtMassChangeStatus;

	/// Helper empty file.
	ptr<File> emptyFile;
	/// Is push in progress.
	bool pushInProgress;
	/// Ids of transient keys.
	std::vector<long long> transientIds;
	/// Number of keys to pull more (estimated).
	long long pullLag;

	//*** Internal help methods.
	/// Check item status for validity.
	void CheckItemStatus(int itemStatus);
	/// Helper method for getting key items.
	KeyItems FillKeyItems(Data::SqliteStatement* stmt);
	/// Get basic info about key.
	KeyItems GetKeyItems(ptr<File> key);
	/// Get basic info about key using id of one of key items.
	KeyItems GetKeyItemsByOneItemId(long long itemId);
	/// Get value of key item.
	ptr<File> GetKeyItemValue(long long itemId);
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

	void DoPush(StreamWriter* writer);

public:
	ClientRepo(const char* fileName);

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

	void Push(StreamWriter* writer);
	void Pull(StreamReader* reader);
	void Cleanup();
};

END_INANITY_OIL

#endif

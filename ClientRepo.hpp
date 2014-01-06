#ifndef ___INANITY_OIL_CLIENT_REPO_HPP___
#define ___INANITY_OIL_CLIENT_REPO_HPP___

#include "Repo.hpp"

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
	struct KeyItems;

	//*** SQLite statements.
	ptr<Data::SqliteStatement> stmtGetKeyItems;
	ptr<Data::SqliteStatement> stmtAddKeyItem;
	ptr<Data::SqliteStatement> stmtRemoveKeyItem;
	ptr<Data::SqliteStatement> stmtChangeKeyItemStatus;
	ptr<Data::SqliteStatement> stmtChangeKeyItemValue;

	//*** Internal help methods. All should be called in transaction.
	/// Check item status for validity.
	void CheckItemStatus(int itemStatus);
	/// Get basic info about key.
	KeyItems GetKeyItems(ptr<File> key);
	/// Add new key item (replacing old one if exists).
	void AddKeyItem(ptr<File> key, ptr<File> value, int status, long long revision);
	/// Remove key item.
	void RemoveKeyItem(long long itemId);
	/// Change key item status (replacing old item if exists).
	void ChangeKeyItemStatus(long long itemId, int newItemStatus);
	/// Change key item value.
	void ChangeKeyItemValue(long long itemId, ptr<File> newValue);

public:
	ClientRepo(const char* fileName);

	/// Get key status.
	KeyStatus GetKeyStatus(ptr<File> key);
	/// Get client value.
	ptr<File> GetValue(ptr<File> key);
	/// Add client change.
	/** If the key is conflicted, just change the client value. */
	void Change(ptr<File> key, ptr<File> value);
	/// Resolve conflicting change.
	/** In case of no conflict throws.
	Just set that client change is not conflicted anymore. */
	void Resolve(ptr<File> key);
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
};

END_INANITY_OIL

#endif

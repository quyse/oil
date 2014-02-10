#ifndef ___INANITY_OIL_ENTITY_HPP___
#define ___INANITY_OIL_ENTITY_HPP___

#include "EntityManager.hpp"
#include "../inanity/script/script.hpp"
#include <vector>

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

class EntityManager;
class EntityScheme;
class EntityCallback;
class Action;

/// Class of an entity.
/** Entity is a medium piece of information stored in repo.
Entity corresponds to some key prefix, and changes to all records
located under this prefix are reported to the entity. */
class Entity : public Object
{
public:
	class FieldEnumerator
	{
	public:
		virtual void OnField(const String& fieldId, ptr<File> value) = 0;
	};

	class DataEnumerator
	{
	public:
		virtual void OnData(ptr<File> key, ptr<File> value) = 0;
	};

private:
	ptr<EntityManager> manager;
	EntityId id;
	/// Scheme of data.
	/** Null if entity doesn't exist. */
	ptr<EntityScheme> scheme;
	/// Callbacks.
	std::vector<EntityCallback*> callbacks;

	ptr<File> GetFullTagKey(const EntityTagId& tagId) const;
	ptr<File> GetFullFieldKey(const String& fieldId) const;
	ptr<File> GetFullDataKey(const void* nameData, size_t nameSize) const;

public:
	Entity(ptr<EntityManager> manager, const EntityId& id);
	~Entity();

	ptr<EntityManager> GetManager() const;
	EntityId GetId() const;

	ptr<EntityScheme> GetScheme() const;
	void SetScheme(ptr<EntityScheme> scheme);

	void OnNewCallback(EntityCallback* callback);
	void OnFreeCallback(EntityCallback* callback);

	void OnChange(const void* keyData, size_t keySize, ptr<File> value);

	ptr<File> ReadTag(const EntityTagId& tagId) const;
	void WriteTag(ptr<Action> action, const EntityTagId& tagId, ptr<File> tagData);

	ptr<File> RawReadField(const String& fieldId) const;
	void RawWriteField(ptr<Action> action, const String& fieldId, ptr<File> value);
	void EnumerateFields(FieldEnumerator* enumerator);
	ptr<Script::Any> ReadField(const String& fieldId) const;
	void WriteField(ptr<Action> action, const String& fieldId, ptr<Script::Any> value);

	ptr<File> ReadData(const void* nameData, size_t nameSize) const;
	void WriteData(ptr<Action> action, const void* nameData, size_t nameSize, ptr<File> value);
	void EnumerateData(DataEnumerator* enumerator);

	ptr<EntityCallback> AddCallback(ptr<Script::Any> callback);
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_ENTITY_HPP___
#define ___INANITY_OIL_ENTITY_HPP___

#include "EntityManager.hpp"
#include "../inanity/script/script.hpp"
#include <vector>
#include <map>

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

class EntityManager;
class EntityScheme;
class EntityCallback;
class EntityInterface;
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
		virtual void OnField(const EntityFieldId& fieldId, ptr<File> value) = 0;
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

	typedef std::map<EntityInterfaceId, EntityInterface*> Interfaces;
	Interfaces interfaces;

	ptr<File> GetFullTagKey(const EntityTagId& tagId) const;
	ptr<File> GetFullFieldKey(const EntityFieldId& fieldId) const;
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

	ptr<File> RawReadField(const EntityFieldId& fieldId) const;
	void RawWriteField(ptr<Action> action, const EntityFieldId& fieldId, ptr<File> value);
	void EnumerateFields(FieldEnumerator* enumerator);
	ptr<Script::Any> ReadField(const EntityFieldId& fieldId) const;
	void WriteField(ptr<Action> action, const EntityFieldId& fieldId, ptr<Script::Any> value);

	ptr<File> RawReadData(const void* nameData, size_t nameSize) const;
	void RawWriteData(ptr<Action> action, const void* nameData, size_t nameSize, ptr<File> value);
	void EnumerateData(DataEnumerator* enumerator);
	ptr<File> ReadData(ptr<File> name) const;
	void WriteData(ptr<Action> action, ptr<File> name, ptr<File> value);

	/// Deletes an entity.
	/** All things (tags, fields, etc.) deleted. But only if entity exists. */
	void Delete(ptr<Action> action);

	ptr<EntityCallback> AddCallback(ptr<Script::Any> callback);

	ptr<EntityInterface> GetInterface(const EntityInterfaceId& interfaceId);
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_ENTITY_HPP___
#define ___INANITY_OIL_ENTITY_HPP___

#include "EntityManager.hpp"
#include "../inanity/script/np/np.hpp"

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_NP

class Any;

END_INANITY_NP

BEGIN_INANITY_OIL

class EntityManager;
class EntityScheme;
class Action;

/// Class of an entity.
/** Entity is a medium piece of information stored in repo.
Entity corresponds to some key prefix, and changes to all records
located under this prefix are reported to the entity. */
class Entity : public Object
{
public:
	//*** Standard tags.
	static EntityTagId tagName;
	static EntityTagId tagDescription;
	static EntityTagId tagTags;

	class FieldEnumerator
	{
	public:
		virtual void OnField(size_t fieldIndex, const void* data, size_t size) = 0;
	};

private:
	ptr<EntityManager> manager;
	EntityId id;
	/// Scheme of data.
	/** Null if entity doesn't exist. */
	ptr<EntityScheme> scheme;
	/// Callback for monitoring data existence and tags.
	ptr<Script::Np::Any> callback;

	ptr<File> GetFullTagKey(const EntityTagId& tagId) const;
	ptr<File> GetFullFieldKey(int fieldIndex) const;
	/// Tries to parse field key.
	/** Returns -1 if wrong format. */
	static int TryParseFieldKey(const void* data, size_t size);

	void FireTagCallback(const EntityTagId& tagId, ptr<File> value);
	void FireFieldCallback(int fieldIndex, ptr<File> value);
	void FireDataCallback(ptr<File> key, ptr<File> value);

public:
	Entity(ptr<EntityManager> manager, const EntityId& id);
	~Entity();

	ptr<EntityManager> GetManager() const;
	EntityId GetId() const;

	ptr<EntityScheme> GetScheme() const;
	void SetScheme(ptr<EntityScheme> scheme);

	void OnChange(const void* keyData, size_t keySize, ptr<File> value);

	ptr<File> ReadTag(const EntityTagId& tagId) const;
	void WriteTag(ptr<Action> action, const EntityTagId& tagId, ptr<File> tagData);

	ptr<File> ReadField(size_t fieldIndex) const;
	void WriteField(ptr<Action> action, size_t fieldIndex, ptr<File> data);
	void EnumerateFields(FieldEnumerator* enumerator);

	void SetCallback(ptr<Script::Any> callback);
};

END_INANITY_OIL

#endif

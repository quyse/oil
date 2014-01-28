#ifndef ___INANITY_OIL_ENTITY_HPP___
#define ___INANITY_OIL_ENTITY_HPP___

#include "EntityId.hpp"

BEGIN_INANITY_OIL

class EntityManager;
class EntityData;

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

private:
	ptr<EntityManager> manager;
	EntityId id;
	/// Entity data.
	/** Could be null even when exists = true. */
	ptr<EntityData> data;

public:
	Entity(ptr<EntityManager> manager, const EntityId& id);
	~Entity();

	ptr<EntityManager> GetManager() const;
	EntityId GetId() const;
	ptr<EntityData> GetData() const;
	void SetData(ptr<EntityData> data);

	ptr<File> GetTag(const EntityTagId& tagId) const;
	void SetTag(const EntityTagId& tagId, ptr<File> tagData);
};

END_INANITY_OIL

#endif

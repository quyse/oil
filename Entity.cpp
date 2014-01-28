#include "Entity.hpp"
#include "EntityManager.hpp"
#include "EntityData.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

EntityTagId Entity::tagName =
	EntityTagId::FromString("0cc78838-4bc7-4da6-bb4b-c4bfcf0363a1");
EntityTagId Entity::tagDescription =
	EntityTagId::FromString("c6f9edf5-ae60-4a38-8408-5f8af4ec7ef3");
EntityTagId Entity::tagTags =
	EntityTagId::FromString("dc25f310-0e15-4913-a546-d6eee3eb87d0");

Entity::Entity(ptr<EntityManager> manager, const EntityId& id)
: manager(manager), id(id)
{
	manager->OnNewEntity(id, this);
}

Entity::~Entity()
{
	manager->OnFreeEntity(id);
}

ptr<EntityManager> Entity::GetManager() const
{
	return manager;
}

EntityId Entity::GetId() const
{
	return id;
}

ptr<EntityData> Entity::GetData() const
{
	return data;
}

void Entity::SetData(ptr<EntityData> data)
{
	this->data = data;
}

ptr<File> Entity::GetTag(const EntityTagId& tagId) const
{
	return manager->GetEntityTag(id, tagId);
}

void Entity::SetTag(const EntityTagId& tagId, ptr<File> tagData)
{
	manager->SetEntityTag(id, tagId, tagData);
}

END_INANITY_OIL

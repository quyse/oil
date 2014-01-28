#include "EntityManager.hpp"
#include "EntityScheme.hpp"
#include "EntitySchemeManager.hpp"
#include "EntityData.hpp"
#include "ClientRepo.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

EntityManager::EntityManager(ptr<ClientRepo> repo, ptr<EntitySchemeManager> schemeManager)
: repo(repo), schemeManager(schemeManager) {}

void EntityManager::OnChange(
	const void* keyData,
	size_t keySize,
	const void* valueData,
	size_t valueSize)
{
	// if key is too short, nothing have to be done
	if(keySize < EntityId::size)
		return;

	// get an entity id
	EntityId entityId = EntityId::FromData(keyData);

	// find an entity
	Entities::iterator entityIterator = entities.find(entityId);
	// if there is no entity, nothing has to be done
	if(entityIterator == entities.end())
		return;

	ptr<Entity> entity = entityIterator->second;
	ptr<EntityData> entityData = entity->GetData();

	// if it's main entity key (scheme key)
	if(keySize == EntityId::size)
	{
		// value should be entity scheme id and has appropriate size
		// if it not, think like there is no scheme
		ptr<EntityScheme> scheme;
		if(valueSize == EntitySchemeId::size)
			scheme = schemeManager->TryGet(EntitySchemeId::FromData(valueData));

		// if sheme doesn't match
		if(entityData->GetScheme() != scheme)
		{
			if(scheme)
				// recreate data
				CreateData(entity, scheme);
			else
				// delete data
				entity->SetData(nullptr);
		}
	}
	else
	{
		// else change should be transmitted to entity if there is data
		if(entityData)
			entityData->OnChange(
				(const char*)keyData + EntityId::size,
				keySize - EntityId::size,
				valueData, valueSize);
	}
}

void EntityManager::OnNewEntity(const EntityId& entityId, Entity* entity)
{
	THROW_ASSERT(entities.find(entityId) == entities.end());

	entities.insert(std::make_pair(entityId, entity));
}

void EntityManager::OnFreeEntity(const EntityId& entityId)
{
	Entities::iterator i = entities.find(entityId);
	THROW_ASSERT(i != entities.end());
	entities.erase(i);
}

void EntityManager::CreateData(Entity* entity, EntityScheme* scheme)
{
	entity->SetData(scheme->CreateData(repo, entity->GetId()));
}

ptr<File> EntityManager::GetEntityTagKey(const EntityId& entityId, const EntityTagId& tagId)
{
	ptr<MemoryFile> key = NEW(MemoryFile(EntityId::size + 1 + EntityTagId::size));
	char* keyData = (char*)key->GetData();
	memcpy(keyData, entityId.data, EntityId::size);
	keyData[EntityId::size] = 't';
	memcpy(keyData + EntityId::size + 1, tagId.data, EntityTagId::size);
	return key;
}

ptr<Entity> EntityManager::CreateEntity(const EntitySchemeId& schemeId)
{
	BEGIN_TRY();

	// get entity scheme
	ptr<EntityScheme> scheme = schemeManager->Get(schemeId);

	// generate new entity id
	EntityId entityId = EntityId::New();

	// create entity
	ptr<Entity> entity = NEW(Entity(this, entityId));

	// create data
	CreateData(entity, scheme);

	return entity;

	END_TRY("Can't create entity");
}

ptr<Entity> EntityManager::GetEntity(const EntityId& entityId)
{
	BEGIN_TRY();

	// try to find an entity
	Entities::const_iterator i = entities.find(entityId);
	if(i != entities.end())
		return i->second;

	// so there is no entity
	// create new
	ptr<Entity> entity = NEW(Entity(this, entityId));

	// read scheme key
	ptr<File> schemeIdFile = repo->GetValue(entityId.ToFile());
	// try get scheme
	ptr<EntityScheme> scheme;
	if(schemeIdFile && schemeIdFile->GetSize() == EntitySchemeId::size)
		scheme = schemeManager->TryGet(EntitySchemeId::FromData(schemeIdFile->GetData()));

	// if there is a scheme, create data
	if(scheme)
		CreateData(entity, scheme);

	return entity;

	END_TRY("Can't get entity");
}

ptr<File> EntityManager::GetEntityTag(const EntityId& entityId, const EntityTagId& tagId)
{
	return repo->GetValue(GetEntityTagKey(entityId, tagId));
}

void EntityManager::SetEntityTag(const EntityId& entityId, const EntityTagId& tagId, ptr<File> tagData)
{
	repo->Change(GetEntityTagKey(entityId, tagId), tagData);
}

END_INANITY_OIL

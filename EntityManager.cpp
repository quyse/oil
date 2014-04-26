#include "EntityManager.hpp"
#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntitySchemeManager.hpp"
#include "ClientRepo.hpp"
#include "Action.hpp"
#include "../inanity/File.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

EntityManager::EntityManager(ptr<ClientRepo> repo, ptr<EntitySchemeManager> schemeManager)
: repo(repo), schemeManager(schemeManager) {}

ptr<ClientRepo> EntityManager::GetRepo() const
{
	return repo;
}

ptr<EntitySchemeManager> EntityManager::GetSchemeManager() const
{
	return schemeManager;
}

void EntityManager::OnChange(ptr<File> key, ptr<File> value)
{
	size_t keySize = key->GetSize();

	// if key is too short, nothing have to be done
	if(keySize < EntityId::size)
		return;

	const char* keyData = (const char*)key->GetData();

	// get an entity id
	EntityId entityId = EntityId::FromData(keyData);

	// find an entity
	Entities::iterator entityIterator = entities.find(entityId);
	// if there is no entity, nothing has to be done
	if(entityIterator == entities.end())
		return;

	// transmit change to the entity
	entityIterator->second->OnChange(
		keyData + EntityId::size,
		keySize - EntityId::size,
		value);
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

ptr<Entity> EntityManager::CreateEntity(ptr<Action> action, const EntitySchemeId& schemeId)
{
	BEGIN_TRY();

	// get entity scheme
	ptr<EntityScheme> scheme = schemeManager->GetScheme(schemeId);

	// generate new entity id
	EntityId entityId = EntityId::New();

	// create entity
	ptr<Entity> entity = NEW(Entity(this, entityId));

	// set scheme
	entity->SetScheme(scheme);

	// add change to action
	action->AddChange(entityId.ToFile(), schemeId.ToFile());

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
		scheme = schemeManager->TryGetScheme(EntitySchemeId::FromData(schemeIdFile->GetData()));

	entity->SetScheme(scheme);

	return entity;

	END_TRY("Can't get entity");
}

END_INANITY_OIL

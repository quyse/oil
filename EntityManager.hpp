#ifndef ___INANITY_OIL_ENTITY_MANAGER_HPP___
#define ___INANITY_OIL_ENTITY_MANAGER_HPP___

#include "EntityId.hpp"
#include <map>

BEGIN_INANITY_OIL

class ClientRepo;
class Entity;
class EntityScheme;
class EntitySchemeManager;

class EntityManager : public Object
{
private:
	ptr<ClientRepo> repo;
	ptr<EntitySchemeManager> schemeManager;

	typedef std::map<EntityId, Entity*> Entities;
	Entities entities;

public:
	EntityManager(ptr<ClientRepo> repo, ptr<EntitySchemeManager> schemeManager);

	ptr<ClientRepo> GetRepo() const;
	ptr<EntitySchemeManager> GetSchemeManager() const;

	void OnChange(ptr<File> key, ptr<File> value);

	/// Method called by entity's constructor.
	void OnNewEntity(const EntityId& entityId, Entity* entity);
	/// Method called by entity's destructor.
	void OnFreeEntity(const EntityId& entityId);

	/// Creates new entity (with generated id) in repo.
	ptr<Entity> CreateEntity(const EntitySchemeId& schemeId);
	/// Gets an entity.
	/** Works always, even if no entity in repo (in that case returns
	entity with no data). */
	ptr<Entity> GetEntity(const EntityId& entityId);
};

END_INANITY_OIL

#endif

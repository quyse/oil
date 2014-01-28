#ifndef ___INANITY_OIL_RECURSIVE_ENTITY_HPP___
#define ___INANITY_OIL_RECURSIVE_ENTITY_HPP___

#include "Entity.hpp"
#include <map>

BEGIN_INANITY_OIL

class ClientRepo;
class EntityScheme;
class EntitySchemeManager;

class EntityManager : public Object
{
private:
	ptr<ClientRepo> repo;
	ptr<EntitySchemeManager> schemeManager;

	typedef std::map<EntityId, Entity*> Entities;
	Entities entities;

	void CreateData(Entity* entity, EntityScheme* scheme);

	static ptr<File> GetEntityTagKey(const EntityId& entityId, const EntityTagId& tagId);

public:
	EntityManager(ptr<ClientRepo> repo, ptr<EntitySchemeManager> schemeManager);

	void OnChange(
		const void* keyData, size_t keySize,
		const void* valueData, size_t valueSize);

	/// Method called by entity's constructor.
	void OnNewEntity(const EntityId& entityId, Entity* entity);
	/// Method called by entity's destructor.
	void OnFreeEntity(const EntityId& entityId);

	ptr<Entity> CreateEntity(const EntitySchemeId& schemeId);
	/// Gets an entity.
	/** Works always, even if no entity in repo (in that case returns
	entity with no data). */
	ptr<Entity> GetEntity(const EntityId& entityId);

	/// Read entity tag without loading entity.
	ptr<File> GetEntityTag(const EntityId& entityId, const EntityTagId& tagId);
	/// Write entity tag.
	void SetEntityTag(const EntityId& entityId, const EntityTagId& tagId, ptr<File> tagData);
};

END_INANITY_OIL

#endif

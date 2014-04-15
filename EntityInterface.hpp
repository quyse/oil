#ifndef ___INANITY_OIL_ENTITY_INTERFACE_HPP___
#define ___INANITY_OIL_ENTITY_INTERFACE_HPP___

#include "Id.hpp"
#include "../inanity/script/script.hpp"
#include <vector>

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

class Entity;
class EntityInterfaceCallback;

/// Entity interface object.
/** Represents an object on interface level. */
class EntityInterface : public Object
{
private:
	ptr<Entity> entity;
	EntityInterfaceId interfaceId;

	/// Callbacks.
	std::vector<EntityInterfaceCallback*> callbacks;

public:
	EntityInterface(ptr<Entity> entity, const EntityInterfaceId& interfaceId);

	ptr<Entity> GetEntity() const;
	EntityInterfaceId GetInterfaceId() const;

	void OnNewCallback(EntityInterfaceCallback* callback);
	void OnFreeCallback(EntityInterfaceCallback* callback);

	ptr<EntityInterfaceCallback> AddCallback(ptr<Script::Any> callback);
};

END_INANITY_OIL

#endif

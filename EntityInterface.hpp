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
class EntityScheme;
class EntityInterfaceCallback;

/// Entity interface object.
/** Represents an object on interface level. */
class EntityInterface : public Object
{
private:
	ptr<Entity> entity;
	EntityInterfaceId interfaceId;
	/// Callback to free interface object.
	ptr<Script::Any> freeInterfaceObject;
	/// Result data.
	ptr<Script::Any> result;
	/// Has result been set at least one time.
	bool resultHasSet;

	/// Callbacks.
	std::vector<EntityInterfaceCallback*> callbacks;

	void FreeInterfaceObject();

public:
	EntityInterface(ptr<Entity> entity, const EntityInterfaceId& interfaceId);
	~EntityInterface();

	ptr<Entity> GetEntity() const;
	EntityInterfaceId GetInterfaceId() const;

	// Notification methods called by EntityInterfaceCallback.
	void OnNewCallback(EntityInterfaceCallback* callback);
	void OnFreeCallback(EntityInterfaceCallback* callback);

	/// Notify about changed result.
	/** Called by Entity. */
	void SetResult(ptr<Script::Any> result);
	/// Get current result.
	ptr<Script::Any> GetResult() const;

	void OnChangeScheme(ptr<EntityScheme> entityScheme);

	ptr<EntityInterfaceCallback> AddCallback(ptr<Script::Any> callback);
};

END_INANITY_OIL

#endif

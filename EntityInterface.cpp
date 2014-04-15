#include "EntityInterface.hpp"
#include "Entity.hpp"
#include "EntityInterfaceCallback.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

EntityInterface::EntityInterface(ptr<Entity> entity, const EntityInterfaceId& interfaceId)
: entity(entity), interfaceId(interfaceId) {}

ptr<Entity> EntityInterface::GetEntity() const
{
	return entity;
}

EntityInterfaceId EntityInterface::GetInterfaceId() const
{
	return interfaceId;
}

void EntityInterface::OnNewCallback(EntityInterfaceCallback* callback)
{
	callbacks.push_back(callback);
}

void EntityInterface::OnFreeCallback(EntityInterfaceCallback* callback)
{
	for(size_t i = 0; i < callbacks.size(); ++i)
		if(callbacks[i] == callback)
		{
			callbacks.erase(callbacks.begin() + i);
			return;
		}

	THROW("Entity interface callback already freed");
}

ptr<EntityInterfaceCallback> EntityInterface::AddCallback(ptr<Script::Any> callback)
{
	return NEW(EntityInterfaceCallback(this, callback.FastCast<Script::Np::Any>()));
}

END_INANITY_OIL

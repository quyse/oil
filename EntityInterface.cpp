#include "EntityInterface.hpp"
#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityInterfaceCallback.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

EntityInterface::EntityInterface(ptr<Entity> entity, const EntityInterfaceId& interfaceId)
: entity(entity), interfaceId(interfaceId)
{
	entity->OnNewInterface(this);
	OnChangeScheme(entity->GetScheme());
}

EntityInterface::~EntityInterface()
{
	try
	{
		FreeInterfaceObject();
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
	}
	entity->OnFreeInterface(this);
}

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

void EntityInterface::SetResult(ptr<Script::Any> result)
{
	this->result = result;
	for(size_t i = 0; i < callbacks.size(); ++i)
		callbacks[i]->Fire();
}

ptr<Script::Any> EntityInterface::GetResult() const
{
	return result;
}

void EntityInterface::OnChangeScheme(ptr<EntityScheme> entityScheme)
{
	FreeInterfaceObject();

	if(entityScheme)
	{
		const EntityScheme::Interfaces& interfaces = entityScheme->GetInterfaces();
		EntityScheme::Interfaces::const_iterator i = interfaces.find(interfaceId);
		if(i != interfaces.end())
			freeInterfaceObject = i->second.callback->Call(
				i->second.callback.FastCast<Script::Np::Any>()->GetState()->WrapObject(entity));
	}
}

ptr<EntityInterfaceCallback> EntityInterface::AddCallback(ptr<Script::Any> callback)
{
	return NEW(EntityInterfaceCallback(this, callback.FastCast<Script::Np::Any>()));
}

void EntityInterface::FreeInterfaceObject()
{
	if(freeInterfaceObject)
	{
		// ensure reentrancy
		ptr<Script::Any> copy = freeInterfaceObject;
		freeInterfaceObject = nullptr;
		copy->Call();
	}
}

END_INANITY_OIL

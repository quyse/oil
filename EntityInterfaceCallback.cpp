#include "EntityInterfaceCallback.hpp"
#include "EntityInterface.hpp"
#include "../inanity/script/Any.hpp"

BEGIN_INANITY_OIL

EntityInterfaceCallback::EntityInterfaceCallback(ptr<EntityInterface> entityInterface, ptr<Script::Any> callback)
: entityInterface(entityInterface), callback(callback)
{
	entityInterface->OnNewCallback(this);
}

EntityInterfaceCallback::~EntityInterfaceCallback()
{
	entityInterface->OnFreeCallback(this);
}

void EntityInterfaceCallback::Fire()
{
	ptr<Script::Any> result = entityInterface->GetResult();
	if(result)
		callback->Call(result);
	else
		callback->Call();
}

END_INANITY_OIL

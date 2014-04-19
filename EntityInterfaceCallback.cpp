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

ptr<Script::Any> EntityInterfaceCallback::GetResult() const
{
	return entityInterface->GetResult();
}

void EntityInterfaceCallback::SetResult(ptr<Script::Any> result)
{
	callback->Call(result);
}

END_INANITY_OIL

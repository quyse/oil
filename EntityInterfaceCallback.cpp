#include "EntityInterfaceCallback.hpp"
#include "EntityInterface.hpp"
#include "../inanity/script/np/Any.hpp"

BEGIN_INANITY_OIL

EntityInterfaceCallback::EntityInterfaceCallback(ptr<EntityInterface> entityInterface, ptr<Script::Np::Any> callback)
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
}

END_INANITY_OIL

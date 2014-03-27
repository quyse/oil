#include "EntityInterface.hpp"

BEGIN_INANITY_OIL

EntityInterface::EntityInterface(const EntityInterfaceId& id, const String& name)
: id(id), name(name) {}

EntityInterfaceId EntityInterface::GetId() const
{
	return id;
}

String EntityInterface::GetName() const
{
	return name;
}

END_INANITY_OIL

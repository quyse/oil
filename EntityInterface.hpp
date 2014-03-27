#ifndef ___INANITY_OIL_ENTITY_INTERFACE_HPP___
#define ___INANITY_OIL_ENTITY_INTERFACE_HPP___

#include "Id.hpp"

BEGIN_INANITY_OIL

class EntityInterface : public Object
{
private:
	EntityInterfaceId id;
	String name;

public:
	EntityInterface(const EntityInterfaceId& id, const String& name);

	EntityInterfaceId GetId() const;
	String GetName() const;
};

END_INANITY_OIL

#endif

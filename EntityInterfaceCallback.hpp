#ifndef ___INANITY_OIL_ENTITY_INTERFACE_CALLBACK_HPP___
#define ___INANITY_OIL_ENTITY_INTERFACE_CALLBACK_HPP___

#include "Id.hpp"
#include "../inanity/script/np/np.hpp"

BEGIN_INANITY_NP

class Any;

END_INANITY_NP

BEGIN_INANITY_OIL

class EntityInterface;

class EntityInterfaceCallback : public Object
{
private:
	ptr<EntityInterface> entityInterface;
	/// Callback for receiving changes.
	/** function(hasInterfaceObject, interfaceObject) */
	ptr<Script::Np::Any> callback;

public:
	EntityInterfaceCallback(ptr<EntityInterface> entityInterface, ptr<Script::Np::Any> callback);
	~EntityInterfaceCallback();

	void Fire();
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_ENTITY_INTERFACE_CALLBACK_HPP___
#define ___INANITY_OIL_ENTITY_INTERFACE_CALLBACK_HPP___

#include "Id.hpp"
#include "../inanity/script/script.hpp"

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

class EntityInterface;

class EntityInterfaceCallback : public Object
{
private:
	ptr<EntityInterface> entityInterface;
	/// Callback for receiving changes.
	/** function(hasInterfaceObject, interfaceObject) */
	ptr<Script::Any> callback;

public:
	EntityInterfaceCallback(ptr<EntityInterface> entityInterface, ptr<Script::Any> callback);
	~EntityInterfaceCallback();

	ptr<Script::Any> GetResult() const;
	void SetResult(ptr<Script::Any> result);
};

END_INANITY_OIL

#endif

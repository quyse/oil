#ifndef ___INANITY_OIL_ENTITY_FIELD_TYPE_HPP___
#define ___INANITY_OIL_ENTITY_FIELD_TYPE_HPP___

#include "oil.hpp"
#include "../inanity/script/np/np.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_NP

class Any;
class State;

END_INANITY_NP

BEGIN_INANITY_OIL

class EntityFieldType
{
public:
	/// Gets name.
	virtual const char* GetName() const = 0;
	/// Convert value to script.
	/** Returns null if wrong format. */
	virtual ptr<Script::Any> TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value) = 0;
	/// Convert value from script.
	/** Returns null if wrong format. */
	virtual ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value) = 0;
};

END_INANITY_OIL

#endif

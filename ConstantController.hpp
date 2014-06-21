#ifndef ___INANITY_OIL_CONSTANT_CONTROLLER_HPP___
#define ___INANITY_OIL_CONSTANT_CONTROLLER_HPP___

#include "Controller.hpp"

BEGIN_INANITY_OIL

/// Generalized controller returning constant value.
template <typename T>
class ConstantController : public Controller<T>
{
private:
	T constantValue;

public:
	ConstantController(const T& constantValue)
	: constantValue(constantValue) {}

	T Get()
	{
		return constantValue;
	}
};

END_INANITY_OIL

#endif

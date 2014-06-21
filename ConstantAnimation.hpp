#ifndef ___INANITY_OIL_CONSTANT_ANIMATION_HPP___
#define ___INANITY_OIL_CONSTANT_ANIMATION_HPP___

#include "Animation.hpp"

BEGIN_INANITY_OIL

template <typename Value>
class ConstantAnimation : public Animation<Value>
{
private:
	Value constantValue;

public:
	ConstantAnimation(const Value& constantValue)
	: constantValue(constantValue) {}

	Value Get(float key)
	{
		return constantValue;
	}
};

END_INANITY_OIL

#endif

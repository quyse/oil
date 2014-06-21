#ifndef ___INANITY_OIL_ANIMATION_HPP___
#define ___INANITY_OIL_ANIMATION_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

/// Generalized abstract class of animation.
/** Given a key (time), provides a value. */
template <typename Value>
class Animation : public Object
{
public:
	virtual Value Get(float key) = 0;
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_ANIMATION_CONTROLLER_HPP___
#define ___INANITY_OIL_ANIMATION_CONTROLLER_HPP___

#include "Controller.hpp"

BEGIN_INANITY_OIL

/// Controller combining animation and key controller.
template <typename T, typename Key = float>
class AnimationController : public Controller<T>
{
private:
	ptr<Animation<T, Key> > animation;
	ptr<Controller<Key> > keyController;

public:
	AnimationController(ptr<Animation<T, Key> > animation, ptr<Controller<Key> > keyController)
	: animation(animation), keyController(keyController) {}

	//*** Controller's methods.
	T Get()
	{
		return animation->Get(keyController->Get());
	}
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_CONTROLLER_HPP___
#define ___INANITY_OIL_CONTROLLER_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_OIL

template <typename T>
class ConstantController;

/// General abstract controller.
/** Represents a source of some sort of data. */
template <typename T>
class Controller : public Object
{
public:
	virtual T Get() = 0;

	/// Create constant controller for this type of data.
	static ptr<Controller<T> > CreateConstant(const T& constantValue)
	{
		return NEW(ConstantController<T>(constantValue));
	}
};

//*** some specializations
typedef Controller<Graphics::vec3> Vec3Controller;
typedef Controller<Graphics::vec4> Vec4Controller;
typedef Controller<Graphics::quat> QuatController;
typedef Controller<Graphics::mat4x4> Mat4x4Controller;

//*** renamed specializations
typedef Vec3Controller PositionController;
typedef QuatController OrientationController;
typedef Vec3Controller ScaleController;
typedef Mat4x4Controller TransformController;
typedef Vec4Controller ColorController;

END_INANITY_OIL

#endif

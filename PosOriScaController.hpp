#ifndef ___INANITY_OIL_POSORISCA_CONTROLLER_HPP___
#define ___INANITY_OIL_POSORISCA_CONTROLLER_HPP___

#include "Controller.hpp"

BEGIN_INANITY_OIL

/// Position-orientation-scale controller.
/** Combines */
class PosOriScaController : public TransformController
{
private:
	ptr<PositionController> positionController;
	ptr<OrientationController> orientationController;
	ptr<ScaleController> scaleController;

public:
	PosOriScaController(
		ptr<PositionController> positionController,
		ptr<OrientationController> orientationController,
		ptr<ScaleController> scaleController
	);
	~PosOriScaController();

	//*** TransformController's methods.
	Math::mat4x4 Get();
};

END_INANITY_OIL

#endif

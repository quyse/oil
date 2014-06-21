#include "PosOriScaController.hpp"

BEGIN_INANITY_OIL

PosOriScaController::PosOriScaController(
	ptr<PositionController> positionController,
	ptr<OrientationController> orientationController,
	ptr<ScaleController> scaleController
) :
	positionController(positionController),
	orientationController(orientationController),
	scaleController(scaleController)
{}

PosOriScaController::~PosOriScaController() {}

Math::mat4x4 PosOriScaController::Get()
{
	return
		Graphics::CreateTranslationMatrix(positionController->Get()) *
		Graphics::QuaternionToMatrix(orientationController->Get()) *
		Graphics::CreateScalingMatrix(scaleController->Get());
}

END_INANITY_OIL

#include "SequenceAnimation.hpp"

BEGIN_INANITY_OIL

#define N(...) \
	template class SequenceAnimation<__VA_ARGS__>

N(StepInterpolator<float>);
N(StepInterpolator<Math::vec2>);
N(StepInterpolator<Math::vec3>);
N(StepInterpolator<Math::vec4>);
N(StepInterpolator<Math::quat>);
N(StepInterpolator<Math::mat4x4>);

N(LinearInterpolator<float>);
N(LinearEigenInterpolator<Math::vec2>);
N(LinearEigenInterpolator<Math::vec3>);
N(LinearEigenInterpolator<Math::vec4>);
N(SlerpInterpolator<float>);
N(LinearEigenInterpolator<Math::mat4x4>);

N(CubicBezierInterpolator<float>);
N(CubicBezierEigenInterpolator<Math::vec2>);
N(CubicBezierEigenInterpolator<Math::vec3>);
N(CubicBezierEigenInterpolator<Math::vec4>);
N(CubicBezierQuatInterpolator<float>);
N(CubicBezierEigenInterpolator<Math::mat4x4>);

#undef N

END_INANITY_OIL

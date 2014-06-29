#ifndef ___INANITY_OIL_SEQUENCE_ANIMATION_HPP___
#define ___INANITY_OIL_SEQUENCE_ANIMATION_HPP___

#include "Animation.hpp"
#include "../inanity/graphics/graphics.hpp"
#include "../inanity/Exception.hpp"
#include <vector>

BEGIN_INANITY_OIL

/// Generalized animation with keyframed values.
/** Stores generalized Data, which should produce values.
Interpolator's approx. definition:
class Interpolator {
public:
	typedef ... Value;
	typedef ... Data;
	Value Convert(Data data) const;
	Value Interpolate(float key1, Data data1, float key2, Data data2, float key) const;
};
*/
template <typename Interpolator>
class SequenceAnimation : public Animation<typename Interpolator::Value>
{
public:
	typedef typename Interpolator::Value Value;
	typedef typename Interpolator::Data Data;
	typedef std::vector<std::pair<float, Data> > Sequence;

private:
	Sequence sequence;
	Interpolator interpolator;

public:
	SequenceAnimation(const Sequence& sequence, const Interpolator& interpolator = Interpolator())
	: sequence(sequence), interpolator(interpolator)
	{
		if(sequence.empty())
			THROW("Can't create sequence animation with empty sequence");
	}

	//*** Animation's methods.
	Value Get(float key)
	{
		// find a last key which less or equal to the given key
		size_t left = 0, right = sequence.size();
		while(left + 1 < right)
		{
			size_t mid = (left + right) / 2;
			if(key < sequence[mid].first)
				right = mid;
			else
				left = mid;
		}

		if(key < sequence[left].first)
		{
			THROW_ASSERT(left == 0);
			return interpolator.Convert(sequence[0].second);
		}
		else if((left + 1) < sequence.size())
			return interpolator.Interpolate(
				sequence[left].first,
				sequence[left].second,
				sequence[left + 1].first,
				sequence[left + 1].second,
				key);
		else
			return interpolator.Convert(sequence[left].second);
	}
};

/// Step interpolator.
template <typename _Value>
class StepInterpolator
{
public:
	typedef _Value Value;
	typedef Value Data;

	Value Convert(Data data) const
	{
		return data;
	}

	Value Interpolate(float key1, Data data1, float key2, Data data2, float key) const
	{
		return data1;
	}
};

/// Linear interpolator.
template <typename _Value>
class LinearInterpolator : public StepInterpolator<_Value>
{
public:
	using typename StepInterpolator<_Value>::Data;

	_Value Interpolate(float key1, Data data1, float key2, Data data2, float key) const
	{
		return Math::lerp(data1, data2, (key - key1) / (key2 - key1));
	}
};

/// Linear Eigen interpolator.
template <typename _Value>
class LinearEigenInterpolator : public StepInterpolator<_Value>
{
public:
	using typename StepInterpolator<_Value>::Data;

	_Value Interpolate(float key1, Data data1, float key2, Data data2, float key) const
	{
		float t = (key - key1) / (key2 - key1);
		return Math::fromEigen((Math::toEigen(data1) * (1 - t) + Math::toEigen(data2) * t).eval());
	}
};

/// Quaternion interpolator.
template <typename T>
class SlerpInterpolator
{
public:
	typedef Math::xquat<T> Value;
	typedef Value Data;

	Value Convert(Data data) const
	{
		return data;
	}

	Value Interpolate(float key1, Data data1, float key2, Data data2, float key) const
	{
		return Math::fromEigen(Math::toEigenQuat(data1).slerp((key - key1) / (key2 - key1), Math::toEigenQuat(data2)));
	}
};

/// Data for bezier interpolator.
template <typename T>
struct CubicBezierFrame
{
	T left;
	T main;
	T right;
};

/// Bezier interpolator.
template <typename _Value>
class CubicBezierInterpolator
{
public:
	typedef _Value Value;
	typedef CubicBezierFrame<Value> Data;

	Value Convert(const Data& data) const
	{
		return data.main;
	}
	Value Interpolate(float key1, const Data& data1, float key2, const Data& data2, float key) const
	{
		float t = (key - key1) / (key2 - key1);
		float t1 = 1 - t;

		Value v01 = data1.main * t1 + data1.right * t;
		Value v12 = data1.right * t1 + data2.left * t;
		Value v23 = data2.left * t1 + data2.main * t;
		Value v012 = v01 * t1 + v12 * t;
		Value v123 = v12 * t1 + v23 * t;
		Value v0123 = v012 * t1 + v123 * t;

		return v0123;
	}
};

/// Bezier Eigen interpolator.
template <typename _Value>
class CubicBezierEigenInterpolator : public CubicBezierInterpolator<_Value>
{
public:
	using typename CubicBezierInterpolator<_Value>::Data;

	_Value Interpolate(float key1, const Data& data1, float key2, const Data& data2, float key) const
	{
		float t = (key - key1) / (key2 - key1);
		float t1 = 1 - t;

		auto v01 = Math::toEigen(data1.main) * t1 + Math::toEigen(data1.right) * t;
		auto v12 = Math::toEigen(data1.right) * t1 + Math::toEigen(data2.left) * t;
		auto v23 = Math::toEigen(data2.left) * t1 + Math::toEigen(data2.main) * t;
		auto v012 = v01 * t1 + v12 * t;
		auto v123 = v12 * t1 + v23 * t;
		auto v0123 = v012 * t1 + v123 * t;

		return Math::fromEigen(v0123.eval());
	}
};

/// Bezier Eigen interpolator for quaternions.
template <typename T>
class CubicBezierQuatInterpolator
{
public:
	typedef Math::xquat<T> Value;
	typedef CubicBezierFrame<Value> Data;

	Value Convert(const Data& data) const
	{
		return data.main;
	}

	Value Interpolate(float key1, const Data& data1, float key2, const Data& data2, float key) const
	{
		float t = (key - key1) / (key2 - key1);

		auto v01 = Math::toEigenQuat(data1.main).slerp(t, Math::toEigenQuat(data1.right));
		auto v12 = Math::toEigenQuat(data1.right).slerp(t, Math::toEigenQuat(data2.left));
		auto v23 = Math::toEigenQuat(data2.left).slerp(t, Math::toEigenQuat(data2.main));
		auto v012 = v01.slerp(t, v12);
		auto v123 = v12.slerp(t, v23);
		auto v0123 = v012.slerp(t, v123);

		return Math::fromEigen(v0123);
	}
};

//*** some explicit instantiations

#define N(name, ...) \
	extern template class SequenceAnimation<__VA_ARGS__>; \
	typedef SequenceAnimation<__VA_ARGS__> name

N(StepFloatAnimation, StepInterpolator<float>);
N(StepVec2Animation, StepInterpolator<Math::vec2>);
N(StepVec3Animation, StepInterpolator<Math::vec3>);
N(StepVec4Animation, StepInterpolator<Math::vec4>);
typedef StepVec4Animation StepQuatAnimation;
N(StepMat4x4Animation, StepInterpolator<Math::mat4x4>);

N(LinearFloatAnimation, LinearInterpolator<float>);
N(LinearVec2Animation, LinearEigenInterpolator<Math::vec2>);
N(LinearVec3Animation, LinearEigenInterpolator<Math::vec3>);
N(LinearVec4Animation, LinearEigenInterpolator<Math::vec4>);
N(LinearQuatAnimation, SlerpInterpolator<float>);
N(LinearMat4x4Animation, LinearEigenInterpolator<Math::mat4x4>);

N(CubicBezierFloatAnimation, CubicBezierInterpolator<float>);
N(CubicBezierVec2Animation, CubicBezierEigenInterpolator<Math::vec2>);
N(CubicBezierVec3Animation, CubicBezierEigenInterpolator<Math::vec3>);
N(CubicBezierVec4Animation, CubicBezierEigenInterpolator<Math::vec4>);
N(CubicBezierQuatAnimation, CubicBezierQuatInterpolator<float>);
N(CubicBezierMat4x4Animation, CubicBezierEigenInterpolator<Math::mat4x4>);

#undef N

//*** some additional typedefs

typedef StepVec3Animation StepPositionAnimation;
typedef StepQuatAnimation StepOrientationAnimation;
typedef StepVec3Animation StepScaleAnimation;
typedef StepMat4x4Animation StepTransformAnimation;

typedef LinearVec3Animation LinearPositionAnimation;
typedef LinearQuatAnimation LinearOrientationAnimation;
typedef LinearVec3Animation LinearScaleAnimation;
typedef LinearMat4x4Animation LinearTransformAnimation;

typedef CubicBezierVec3Animation CubicBezierPositionAnimation;
typedef CubicBezierQuatAnimation CubicBezierOrientationAnimation;
typedef CubicBezierVec3Animation CubicBezierScaleAnimation;
typedef CubicBezierMat4x4Animation CubicBezierTransformAnimation;

END_INANITY_OIL

#endif

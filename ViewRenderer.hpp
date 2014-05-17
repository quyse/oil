#ifndef ___INANITY_OIL_VIEW_RENDERER_HPP___
#define ___INANITY_OIL_VIEW_RENDERER_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_GRAPHICS

class FrameBuffer;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

/// Abstract class of view renderer.
/** Represents object controlling rendering into view plugin. */
class ViewRenderer : public Object
{
public:
	/// Perform actual render.
	virtual void Render() = 0;
};

END_INANITY_OIL

#endif

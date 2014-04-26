#ifndef ___INANITY_OIL_PAINTER_HPP___
#define ___INANITY_OIL_PAINTER_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_GRAPHICS

class Device;
class Context;
class ShaderCache;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

class Painter : public Object
{
private:
	ptr<Graphics::Device> device;
	ptr<Graphics::Context> context;
	ptr<Graphics::ShaderCache> shaderCache;

public:
	Painter(ptr<Graphics::Device> device, ptr<Graphics::Context> context, ptr<Graphics::ShaderCache> shaderCache);
};

END_INANITY_OIL

#endif

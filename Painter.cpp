#include "Painter.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/graphics/Context.hpp"
#include "../inanity/graphics/ShaderCache.hpp"

BEGIN_INANITY_OIL

using namespace Inanity::Graphics;
using namespace Inanity::Graphics::Shaders;

Painter::Painter(ptr<Device> device, ptr<Context> context, ptr<ShaderCache> shaderCache)
: device(device), context(context), shaderCache(shaderCache)
{
}

END_INANITY_OIL

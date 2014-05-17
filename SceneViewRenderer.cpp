#include "SceneViewRenderer.hpp"
#include "Engine.hpp"
#include "../inanity/graphics/FrameBuffer.hpp"

BEGIN_INANITY_OIL

SceneViewRenderer::SceneViewRenderer(ptr<Engine> engine)
: engine(engine) {}

SceneViewRenderer::~SceneViewRenderer() {}

void SceneViewRenderer::Render()
{
}

END_INANITY_OIL

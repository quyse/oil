#ifndef ___INANITY_OIL_SCENE_VIEW_RENDERER_HPP___
#define ___INANITY_OIL_SCENE_VIEW_RENDERER_HPP___

#include "ViewRenderer.hpp"

BEGIN_INANITY_OIL

class Engine;

/// View renderer supporting rendering scene with multiple objects.
class SceneViewRenderer : public ViewRenderer
{
private:
	ptr<Engine> engine;

public:
	SceneViewRenderer(ptr<Engine> engine);
	~SceneViewRenderer();

	//*** ViewRenderer's methods.
	void Render();
};

END_INANITY_OIL

#endif

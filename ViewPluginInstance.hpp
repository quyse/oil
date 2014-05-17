#ifndef ___INANITY_OIL_VIEW_PLUGIN_INSTANCE_HPP___
#define ___INANITY_OIL_VIEW_PLUGIN_INSTANCE_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"
#include "../inanity/platform/NpapiPluginInstance.hpp"
#include "../inanity/Ticker.hpp"

BEGIN_INANITY_GRAPHICS

class RenderBuffer;
class FrameBuffer;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

class ViewRenderer;
class ViewScriptObject;

class ViewPluginInstance : public Platform::NpapiPluginInstance
{
private:
	ptr<Graphics::RenderBuffer> renderBuffer;
	ptr<Graphics::FrameBuffer> frameBuffer;
	int renderBufferWidth, renderBufferHeight;
	ptr<ViewRenderer> viewRenderer;

	/// Keep reference to view script object.
	/** NpapiPluginInstance::scriptObject will be reclaimed
	eventually, to workaround a wrapper problem. */
	ptr<ViewScriptObject> viewScriptObject;

	Ticker ticker;

#ifdef ___INANITY_PLATFORM_WINDOWS
	void Paint(HDC hdc);
#endif

	void Draw();

	void PostInit();

public:
	ViewPluginInstance();

	void SetViewRenderer(ptr<ViewRenderer> viewRenderer);
};

END_INANITY_OIL

#endif

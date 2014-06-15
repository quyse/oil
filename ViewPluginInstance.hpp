#ifndef ___INANITY_OIL_VIEW_PLUGIN_INSTANCE_HPP___
#define ___INANITY_OIL_VIEW_PLUGIN_INSTANCE_HPP___

#include "oil.hpp"
#include "../inanity/platform/NpapiPluginInstance.hpp"
#include "../inanity/Ticker.hpp"

BEGIN_INANITY_OIL

class ViewRenderer;
class ViewScriptObject;

class ViewPluginInstance : public Platform::NpapiPluginInstance
{
private:
	ptr<ViewRenderer> viewRenderer;

	/// Keep reference to view script object.
	/** NpapiPluginInstance::scriptObject will be reclaimed
	eventually, to workaround a wrapper problem. */
	ptr<ViewScriptObject> viewScriptObject;

	Ticker ticker;

	ptr<Graphics::Device> GetGraphicsDevice() const;
	void Draw();

	void PostInit();

public:
	ViewPluginInstance();

	void SetViewRenderer(ptr<ViewRenderer> viewRenderer);
};

END_INANITY_OIL

#endif

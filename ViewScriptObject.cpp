#include "ViewScriptObject.hpp"
#include "ViewPluginInstance.hpp"

BEGIN_INANITY_OIL

ViewScriptObject::ViewScriptObject(ViewPluginInstance* pluginInstance)
: pluginInstance(pluginInstance) {}

void ViewScriptObject::Invalidate()
{
	pluginInstance->Invalidate();
}

END_INANITY_OIL

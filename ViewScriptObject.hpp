#ifndef ___INANITY_OIL_VIEW_SCRIPT_OBJECT_HPP___
#define ___INANITY_OIL_VIEW_SCRIPT_OBJECT_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class ViewPluginInstance;

/// Script object for 'view' plugin instance.
class ViewScriptObject : public Object
{
private:
	ViewPluginInstance* pluginInstance;

public:
	ViewScriptObject(ViewPluginInstance* pluginInstance);

	void Invalidate();
};

END_INANITY_OIL

#endif

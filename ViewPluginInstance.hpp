#ifndef ___INANITY_OIL_VIEW_PLUGIN_INSTANCE_HPP___
#define ___INANITY_OIL_VIEW_PLUGIN_INSTANCE_HPP___

#include "oil.hpp"
#include "../inanity/platform/NpapiPluginInstance.hpp"

BEGIN_INANITY_OIL

class ViewPluginInstance : public Platform::NpapiPluginInstance
{
private:
#ifdef ___INANITY_PLATFORM_WINDOWS
	void Paint(HDC hdc);
#endif

public:
	ViewPluginInstance();
};

END_INANITY_OIL

#endif

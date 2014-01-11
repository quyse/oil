#include "ViewPluginInstance.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#ifdef ___INANITY_PLATFORM_WINDOWS
#include "../inanity/platform/Win32Window.hpp"
#include "../inanity/input/Win32WmManager.hpp"
#endif

BEGIN_INANITY_OIL

ViewPluginInstance::ViewPluginInstance()
: NpapiPluginInstance(true)
{
	name = "Inanity Oil NPAPI View Plugin";
	description = name;
	windowless = false;
}

#ifdef ___INANITY_PLATFORM_WINDOWS

void ViewPluginInstance::Paint(HDC hdc)
{
	std::wostringstream ss;
	ss << L"V " << width << L'x' << height;
	TextOut(hdc, 0, 0, ss.str().c_str(), ss.str().length());
}

#endif

END_INANITY_OIL

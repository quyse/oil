#include "ViewPluginInstance.hpp"
#include "../inanity/script/np/State.hpp"
#ifdef ___INANITY_PLATFORM_WINDOWS
#include "../inanity/platform/Win32Window.hpp"
#include "../inanity/input/Win32WmManager.hpp"
#endif

BEGIN_INANITY_OIL

ViewPluginInstance::ViewPluginInstance()
: NpapiPluginInstance(true, false) {}

#ifdef ___INANITY_PLATFORM_WINDOWS

void ViewPluginInstance::Paint(HDC hdc)
{
	TextOut(hdc, 0, 0, L"V", 1);
}

#endif

END_INANITY_OIL

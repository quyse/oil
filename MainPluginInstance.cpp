#include "MainPluginInstance.hpp"
#include "ScriptObject.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#ifdef ___INANITY_PLATFORM_WINDOWS
#include "../inanity/input/Win32WmManager.hpp"
#include "../inanity/platform/Win32Window.hpp"
#endif

BEGIN_INANITY_OIL

MainPluginInstance* MainPluginInstance::instance = nullptr;

MainPluginInstance::MainPluginInstance()
: NpapiPluginInstance(false)
{
	name = "Inanity Oil NPAPI Main Plugin";
	description = name;
	windowless = true;

	instance = this;
}

MainPluginInstance::~MainPluginInstance()
{
	instance = nullptr;
}

ptr<Script::Np::State> MainPluginInstance::GetScriptState() const
{
	return scriptState;
}

#ifdef ___INANITY_PLATFORM_WINDOWS

void MainPluginInstance::Paint(HDC hdc)
{
	TextOut(hdc, 0, 0, L"M", 1);
}

#endif

void MainPluginInstance::PostInit()
{
	scriptState = NEW(Script::Np::State(this));
	scriptObject = scriptState->WrapObject<ScriptObject>(NEW(ScriptObject(scriptState)));
}

END_INANITY_OIL

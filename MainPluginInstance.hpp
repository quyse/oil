#ifndef ___INANITY_OIL_MAIN_PLUGIN_INSTANCE_HPP___
#define ___INANITY_OIL_MAIN_PLUGIN_INSTANCE_HPP___

#include "oil.hpp"
#include "../inanity/platform/NpapiPluginInstance.hpp"
#include "../inanity/input/Manager.hpp"

BEGIN_INANITY_NP

class State;

END_INANITY_NP

BEGIN_INANITY_OIL

class MainPluginInstance : public Platform::NpapiPluginInstance
{
private:
	ptr<Script::Np::State> scriptState;

#ifdef ___INANITY_PLATFORM_WINDOWS
	void Paint(HDC hdc);
#endif

	void PostInit();

public:
	static MainPluginInstance* instance;

public:
	MainPluginInstance();
	~MainPluginInstance();

	ptr<Script::Np::State> GetScriptState() const;
};

END_INANITY_OIL

#endif

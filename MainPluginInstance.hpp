#ifndef ___INANITY_OIL_MAIN_PLUGIN_INSTANCE_HPP___
#define ___INANITY_OIL_MAIN_PLUGIN_INSTANCE_HPP___

#include "oil.hpp"
#include "../inanity/platform/NpapiPluginInstance.hpp"
#include "../inanity/graphics/graphics.hpp"
#include "../inanity/input/Manager.hpp"

BEGIN_INANITY_GRAPHICS

class System;
class Device;

END_INANITY_GRAPHICS

BEGIN_INANITY_NP

class State;

END_INANITY_NP

BEGIN_INANITY_OIL

class MainPluginInstance : public Platform::NpapiPluginInstance
{
private:
	ptr<Script::Np::State> scriptState;

	ptr<Graphics::System> graphicsSystem;
	ptr<Graphics::Device> graphicsDevice;

#ifdef ___INANITY_PLATFORM_WINDOWS
	void Paint(HDC hdc);
#endif

	void PostInit();

public:
	MainPluginInstance();

	ptr<Graphics::Device> GetGraphicsDevice() const;
};

END_INANITY_OIL

#endif

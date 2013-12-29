#include "MainPluginInstance.hpp"
#include "../inanity/graphics/System.hpp"
#include "../inanity/graphics/Adapter.hpp"
#include "../inanity/graphics/Device.hpp"
#include "../inanity/platform/Game.cpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/Exception.hpp"
#ifdef ___INANITY_PLATFORM_WINDOWS
#include "../inanity/input/Win32WmManager.hpp"
#endif

BEGIN_INANITY_OIL

MainPluginInstance* MainPluginInstance::instance = nullptr;

MainPluginInstance::MainPluginInstance()
: NpapiPluginInstance(false, true)
{
	if(instance)
		THROW("Only one main instance of Oil plugin allowed");

	name = "Inanity Oil NPAPI Main Plugin";
	description = name;
	windowless = true;

	graphicsSystem = Platform::Game::CreateDefaultGraphicsSystem();
	ptr<Graphics::Adapter> adapter = graphicsSystem->GetAdapters()[0];
	graphicsDevice = graphicsSystem->CreateDevice(adapter);
}

MainPluginInstance::~MainPluginInstance()
{
	instance = nullptr;
}

MainPluginInstance* MainPluginInstance::GetInstance()
{
	return instance;
}

ptr<Graphics::Device> MainPluginInstance::GetGraphicsDevice() const
{
	return graphicsDevice;
}

#ifdef ___INANITY_PLATFORM_WINDOWS

void MainPluginInstance::Paint(HDC hdc)
{
	TextOut(hdc, 0, 0, L"M", 1);
}

#endif

END_INANITY_OIL

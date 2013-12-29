#include "MainPluginInstance.hpp"
#include "ViewPluginInstance.hpp"
#include "../inanity/platform/NpapiPlugin.hpp"
#include "../inanity/Strings.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY

ptr<Platform::NpapiPluginInstance> Platform::NpapiPlugin::CreateInstance(int argc, char* argn[], char* argv[])
{
	BEGIN_TRY();

	String instanceType;

	for(int i = 0; i < argc; ++i)
	{
		const char* argName = argn[i];
		const char* argValue = argv[i];

		if(strcmp(argName, "oiltype") == 0)
			instanceType = argValue;
	}

	if(instanceType.empty())
		THROW("No instance type is given");

	if(instanceType == "main")
		return NEW(Oil::MainPluginInstance());
	if(instanceType == "view")
		return NEW(Oil::ViewPluginInstance());

	THROW("Unknown instance type");

	END_TRY("Can't create Oil plugin instance");
}

END_INANITY

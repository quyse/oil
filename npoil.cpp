#include "MainPluginInstance.hpp"
#include "ViewPluginInstance.hpp"
#include "../inanity/platform/NpapiPlugin.hpp"
#include "../inanity/Strings.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY

const Platform::NpapiPlugin::Info& Platform::NpapiPlugin::GetInfo()
{
	// information about plugin
	// should be in sync with Windows resource file
	static const Platform::NpapiPlugin::Info info =
	{
		// version
		"1.0.0.1",
		// mime
		"application/x-inanityoil::Inanity Oil NPAPI Plugin",
		// "name"
		"Inanity Oil NPAPI Plugin",
		 // description
		"Inanity Oil NPAPI Plugin"
	};
	return info;
}

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

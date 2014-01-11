#include "ScriptObject.hpp"
#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "MainPluginInstance.hpp"
#include "../inanity/Handler.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/platform/FileSystem.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

//*** class ScriptObject::CheckRepoHandler

class ScriptObject::CheckRepoHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptObject> scriptObject;
	ptr<Script::Any> callback;

public:
	CheckRepoHandler(ptr<ScriptObject> scriptObject, ptr<Script::Any> callback)
	: scriptObject(scriptObject), callback(callback) {}

	void OnData(ptr<File> data)
	{
		try
		{
			ptr<ClientRepo> repo = ClientRepo::CreateInMemory();
			repo->ReadServerManifest(&StreamReader(NEW(FileInputStream(data))));
			callback->Call(
				scriptObject->scriptState->NewBoolean(true),
				scriptObject->scriptState->NewString("OK"));
				//scriptObject->scriptState->NewNumber(123));
		}
		catch(Exception* exception)
		{
			OnError(exception);
		}
	}

	void OnError(ptr<Exception> exception)
	{
		std::ostringstream ss;
		exception->PrintStack(ss);
		callback->Call(
			scriptObject->scriptState->NewBoolean(false),
			scriptObject->scriptState->NewString(ss.str()));
			//scriptObject->scriptState->NewNumber(123));
	}
};

//*** class ScriptObject

ScriptObject::ScriptObject(ptr<Script::Np::State> scriptState)
: scriptState(scriptState)
{
	nativeFileSystem = Platform::FileSystem::GetNativeFileSystem();
}

ptr<Script::Any> ScriptObject::GetRootNamespace() const
{
	return scriptState->GetRootNamespace();
}

ptr<FileSystem> ScriptObject::GetNativeFileSystem() const
{
	return nativeFileSystem;
}

void ScriptObject::CheckRepo(const String& url, ptr<Script::Any> callback)
{
	MainPluginInstance::GetInstance()->GetUrl(
		url + "?manifest",
		NEW(CheckRepoHandler(this, callback)));
}

END_INANITY_OIL

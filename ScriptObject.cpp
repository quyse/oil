#include "ScriptObject.hpp"
#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "ServerRepo.hpp"
#include "LocalRemoteRepo.hpp"
#include "UrlRemoteRepo.hpp"
#include "MainPluginInstance.hpp"
#include "../inanity/Handler.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/platform/FileSystem.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

//*** class ScriptObject::CheckRemoteRepoManifestHandler

class ScriptObject::CheckRemoteRepoManifestHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptObject> scriptObject;
	ptr<Script::Any> callback;

public:
	CheckRemoteRepoManifestHandler(ptr<ScriptObject> scriptObject, ptr<Script::Any> callback)
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

ptr<ClientRepo> ScriptObject::CreateInMemoryClientRepo()
{
	return ClientRepo::CreateInMemory();
}

ptr<ClientRepo> ScriptObject::CreateLocalFileClientRepo(const String& fileName)
{
	return NEW(ClientRepo(fileName.c_str()));
}

ptr<RemoteRepo> ScriptObject::CreateLocalRemoteRepo(const String& fileName)
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(fileName.c_str()))));
}

ptr<RemoteRepo> ScriptObject::CreateUrlRemoteRepo(const String& url)
{
	return NEW(UrlRemoteRepo(url));
}

void ScriptObject::CheckRemoteRepoManifest(ptr<RemoteRepo> remoteRepo, ptr<Script::Any> callback)
{
	remoteRepo->GetManifest(NEW(CheckRemoteRepoManifestHandler(this, callback)));
}

END_INANITY_OIL

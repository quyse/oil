#include "ScriptObject.hpp"
#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "ServerRepo.hpp"
#include "LocalRemoteRepo.hpp"
#include "UrlRemoteRepo.hpp"
#include "EntityManager.hpp"
#include "EntityScheme.hpp"
#include "EntitySchemeManager.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/platform/FileSystem.hpp"
#include "../inanity/MemoryFile.hpp"

// classes only for registration in script state
#include "Action.hpp"
#include "../inanity/Strings.hpp"

BEGIN_INANITY_OIL

ScriptObject::ScriptObject(ptr<Script::Np::State> scriptState)
: scriptState(scriptState)
{
	nativeFileSystem = Platform::FileSystem::GetNativeFileSystem();

	// create entity scheme manager and register schemes
	entitySchemeManager = NEW(EntitySchemeManager());

	// register some classes
	scriptState->Register<Strings>();
	scriptState->Register<Action>();
	scriptState->Register<EntityScheme>();
}

ptr<Script::Any> ScriptObject::GetRootNamespace() const
{
	return scriptState->GetRootNamespace();
}

ptr<FileSystem> ScriptObject::GetNativeFileSystem() const
{
	return nativeFileSystem;
}

ptr<ClientRepo> ScriptObject::CreateLocalClientRepo(const String& fileName)
{
	return NEW(ClientRepo(fileName.c_str()));
}

ptr<ClientRepo> ScriptObject::CreateTempClientRepo()
{
	return NEW(ClientRepo(Repo::fileNameTemp));
}

ptr<ClientRepo> ScriptObject::CreateMemoryClientRepo()
{
	return NEW(ClientRepo(Repo::fileNameMemory));
}

ptr<RemoteRepo> ScriptObject::CreateUrlRemoteRepo(const String& url)
{
	return NEW(UrlRemoteRepo(scriptState->GetPluginInstance(), url));
}

ptr<RemoteRepo> ScriptObject::CreateLocalRemoteRepo(const String& fileName)
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(fileName.c_str()))));
}

ptr<RemoteRepo> ScriptObject::CreateTempRemoteRepo()
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(Repo::fileNameTemp))));
}

ptr<RemoteRepo> ScriptObject::CreateMemoryRemoteRepo()
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(Repo::fileNameMemory))));
}

ptr<ScriptRepo> ScriptObject::CreateScriptRepo(ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo)
{
	return NEW(ScriptRepo(
		clientRepo,
		remoteRepo,
		NEW(EntityManager(clientRepo, entitySchemeManager))));
}

END_INANITY_OIL

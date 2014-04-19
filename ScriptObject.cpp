#include "ScriptObject.hpp"
#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "ServerRepo.hpp"
#include "LocalRemoteRepo.hpp"
#include "UrlRemoteRepo.hpp"
#include "EntityManager.hpp"
#include "EntityScheme.hpp"
#include "EntitySchemeManager.hpp"
#include "Engine.hpp"
#include "EntityFieldTypes.hpp"
#include "FileEntityScheme.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/platform/FileSystem.hpp"
#include "../inanity/data/SQLiteFileSystem.hpp"
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
	scriptState->Register<EntityId>();
	scriptState->Register<EntitySchemeId>();
	scriptState->Register<EntityInterfaceId>();
	scriptState->Register<EntityTagId>();
	scriptState->Register<EntityFieldId>();
	scriptState->Register<EntityScheme>();
	scriptState->Register<ReferenceEntityFieldType>();
	scriptState->Register<FileEntitySchemeInputStream>();
	scriptState->Register<FileEntitySchemeOutputStream>();
}

ptr<Script::Any> ScriptObject::GetRootNamespace() const
{
	return scriptState->GetRootNamespace();
}

ptr<FileSystem> ScriptObject::GetNativeFileSystem() const
{
	return nativeFileSystem;
}

ptr<FileSystem> ScriptObject::GetProfileFileSystem() const
{
	return profileFileSystem;
}

void ScriptObject::SetProfilePath(const String& profilePath)
{
	this->profilePath = profilePath;
	profileFileSystem = NEW(Platform::FileSystem(profilePath));
}

void ScriptObject::Init()
{
	engine = NEW(Engine(NEW(Platform::FileSystem("assets")), NEW(Data::SQLiteFileSystem(profilePath + "/shaders"))));
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

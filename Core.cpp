#include "Core.hpp"
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
#include "ViewScriptObject.hpp"
#include "TextureViewRenderer.hpp"
#include "SceneViewRenderer.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/platform/FileSystem.hpp"
#include "../inanity/data/SQLiteFileSystem.hpp"
#include "../inanity/data/Base64OutputStream.hpp"
#include "../inanity/graphics/BmpImage.hpp"
#include "../inanity/graphics/TgaImageLoader.hpp"
#include "../inanity/MemoryStream.hpp"
#include "../inanity/MemoryFile.hpp"

// classes only for registration in script state
#include "Action.hpp"
#include "../inanity/Strings.hpp"

BEGIN_INANITY_OIL

Core::Core(ptr<Script::Np::State> scriptState)
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
	scriptState->Register<ViewScriptObject>();
	scriptState->Register<TextureViewRenderer>();
	scriptState->Register<SceneViewRenderer>();
	scriptState->Register<Data::Base64OutputStream>();
	scriptState->Register<Graphics::BmpImage>();
	scriptState->Register<Graphics::TgaImageLoader>();
	scriptState->Register<MemoryStream>();
	scriptState->Register<MemoryFile>();
}

ptr<Script::Any> Core::GetRootNamespace() const
{
	return scriptState->GetRootNamespace();
}

ptr<FileSystem> Core::GetNativeFileSystem() const
{
	return nativeFileSystem;
}

ptr<FileSystem> Core::GetProfileFileSystem() const
{
	return profileFileSystem;
}

void Core::SetProfilePath(const String& profilePath)
{
	this->profilePath = profilePath;
	profileFileSystem = NEW(Platform::FileSystem(profilePath));
}

void Core::Init()
{
	engine = NEW(Engine(NEW(Platform::FileSystem("assets")), NEW(Data::SQLiteFileSystem(profilePath + "/shaders"))));
}

ptr<ClientRepo> Core::CreateLocalClientRepo(const String& fileName)
{
	return NEW(ClientRepo(fileName.c_str()));
}

ptr<ClientRepo> Core::CreateTempClientRepo()
{
	return NEW(ClientRepo(Repo::fileNameTemp));
}

ptr<ClientRepo> Core::CreateMemoryClientRepo()
{
	return NEW(ClientRepo(Repo::fileNameMemory));
}

ptr<RemoteRepo> Core::CreateUrlRemoteRepo(const String& url)
{
	return NEW(UrlRemoteRepo(scriptState->GetPluginInstance(), url));
}

ptr<RemoteRepo> Core::CreateLocalRemoteRepo(const String& fileName)
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(fileName.c_str()))));
}

ptr<RemoteRepo> Core::CreateTempRemoteRepo()
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(Repo::fileNameTemp))));
}

ptr<RemoteRepo> Core::CreateMemoryRemoteRepo()
{
	return NEW(LocalRemoteRepo(NEW(ServerRepo(Repo::fileNameMemory))));
}

ptr<ScriptRepo> Core::CreateScriptRepo(ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo)
{
	return NEW(ScriptRepo(
		clientRepo,
		remoteRepo,
		NEW(EntityManager(clientRepo, entitySchemeManager))));
}

ptr<Engine> Core::GetEngine() const
{
	return engine;
}

END_INANITY_OIL

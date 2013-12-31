#include "ScriptObject.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/platform/FileSystem.hpp"

BEGIN_INANITY_OIL

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

END_INANITY_OIL

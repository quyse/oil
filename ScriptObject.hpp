#ifndef ___INANITY_OIL_SCRIPT_OBJECT_HPP___
#define ___INANITY_OIL_SCRIPT_OBJECT_HPP___

#include "oil.hpp"
#include "../inanity/script/script.hpp"
#include "../inanity/script/np/np.hpp"
#include "../inanity/meta/decl.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY

class FileSystem;

END_INANITY

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_NP

class State;

END_INANITY_NP

BEGIN_INANITY_OIL

class ScriptRepo;

class ScriptObject : public Object
{
private:
	ptr<Script::Np::State> scriptState;
	ptr<FileSystem> nativeFileSystem;
	/// Current repo.
	ptr<ScriptRepo> repo;

	class CheckRepoHandler;

public:
	ScriptObject(ptr<Script::Np::State> scriptState);

	ptr<Script::Any> GetRootNamespace() const;
	ptr<FileSystem> GetNativeFileSystem() const;
	void CheckRepo(const String& url, ptr<Script::Any> callback);

	META_DECLARE_CLASS(ScriptObject);
};

END_INANITY_OIL

#endif

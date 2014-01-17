#ifndef ___INANITY_OIL_SCRIPT_REPO_HPP___
#define ___INANITY_OIL_SCRIPT_REPO_HPP___

#include "oil.hpp"
#include "../inanity/script/script.hpp"
#include "../inanity/meta/decl.hpp"

BEGIN_INANITY_SCRIPT

class State;
class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

class ClientRepo;
class RemoteRepo;

/// Class which combines client and remote repos.
/** Mainly intended to interact with script. */
class ScriptRepo : public Object
{
private:
	ptr<Script::State> scriptState;
	ptr<ClientRepo> clientRepo;
	ptr<RemoteRepo> remoteRepo;

	class InitHandler;
	class PullHandler;
	class WatchHandler;

public:
	ScriptRepo(ptr<Script::State> scriptState, ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo);

	void Init(ptr<Script::Any> callback);
	void Sync(ptr<Script::Any> callback);
	void Watch(ptr<Script::Any> callback);

	META_DECLARE_CLASS(ScriptRepo);
};

END_INANITY_OIL

#endif

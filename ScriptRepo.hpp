#ifndef ___INANITY_OIL_SCRIPT_REPO_HPP___
#define ___INANITY_OIL_SCRIPT_REPO_HPP___

#include "oil.hpp"
#include "../inanity/script/np/np.hpp"
#include "../inanity/meta/decl.hpp"
#include <vector>

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_NP

class Any;

END_INANITY_NP

BEGIN_INANITY_OIL

class ClientRepo;
class RemoteRepo;
class EntityManager;
class Action;

/// Class which combines client and remote repos.
/** Mainly intended to interact with script. */
class ScriptRepo : public Object
{
private:
	ptr<ClientRepo> clientRepo;
	ptr<RemoteRepo> remoteRepo;
	ptr<EntityManager> entityManager;

	std::vector<ptr<Action> > undoActions;
	std::vector<ptr<Action> > redoActions;

	ptr<Script::Np::Any> undoRedoChangedCallback;

	class InitHandler;
	class PullHandler;
	class WatchHandler;

	void ApplyAction(ptr<Action> action);
	ptr<Action> ReverseAction(ptr<Action> action);

	void FireUndoRedoChangedCallback();

	void OnChange(ptr<File> key, ptr<File> value);

public:
	ScriptRepo(ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo, ptr<EntityManager> entityManager);

	ptr<EntityManager> GetEntityManager() const;

	void Init(ptr<Script::Any> callback);
	void Sync(ptr<Script::Any> callback);
	void Watch(ptr<Script::Any> callback);

	void ProcessEvents();

	void Change(ptr<File> key, ptr<File> value);
	/// Subscribe for changes in keys with specified prefix.
	/** \param callback function(key, flags) */
	void Subscribe(ptr<File> prefix, ptr<Script::Any> callback);

	/// Make undoable action.
	void MakeAction(ptr<Action> action);
	/// Undo action from stack.
	bool Undo();
	/// Redo action from stack.
	bool Redo();

	void SetUndoRedoChangedCallback(ptr<Script::Any> callback);

	long long GetPushLag() const;
	long long GetPushedKeysCount() const;
	long long GetPullLag() const;
	long long GetPulledKeysCount() const;

	ptr<ClientRepo> GetClientRepo() const;

	META_DECLARE_CLASS(ScriptRepo);
};

END_INANITY_OIL

#endif

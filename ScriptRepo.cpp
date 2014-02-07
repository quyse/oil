#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "RemoteRepo.hpp"
#include "EntityManager.hpp"
#include "Action.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/MemoryStream.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/File.hpp"
#include <sstream>

BEGIN_INANITY_OIL

//*** class ScriptRepo::InitHandler

class ScriptRepo::InitHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptRepo> scriptRepo;
	ptr<Script::Np::Any> callback;

public:
	InitHandler(ptr<ScriptRepo> scriptRepo, ptr<Script::Np::Any> callback)
	: scriptRepo(scriptRepo), callback(callback) {}

	void OnData(ptr<File> data)
	{
		try
		{
			scriptRepo->clientRepo->ReadServerManifest(&StreamReader(NEW(FileInputStream(data))));
			ptr<Script::Np::State> scriptState = callback->GetState();
			callback->Call(
				scriptState->NewBoolean(true),
				scriptState->NewString("OK"));
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
		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewBoolean(false),
			scriptState->NewString(ss.str()));
	}
};

//*** class ScriptRepo::PullHandler

class ScriptRepo::PullHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptRepo> scriptRepo;
	ptr<Script::Np::Any> callback;

public:
	PullHandler(ptr<ScriptRepo> scriptRepo, ptr<Script::Np::Any> callback)
	: scriptRepo(scriptRepo), callback(callback) {}

	void OnData(ptr<File> data)
	{
		try
		{
			bool changedSomething = scriptRepo->clientRepo->Pull(&StreamReader(NEW(FileInputStream(data))));
			ptr<Script::Np::State> scriptState = callback->GetState();
			callback->Call(
				scriptState->NewBoolean(true),
				scriptState->NewBoolean(changedSomething));
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
		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewBoolean(false),
			scriptState->NewString(ss.str()));
	}
};

//*** class ScriptRepo::WatchHandler

class ScriptRepo::WatchHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptRepo> scriptRepo;
	ptr<Script::Np::Any> callback;

public:
	WatchHandler(ptr<ScriptRepo> scriptRepo, ptr<Script::Np::Any> callback)
	: scriptRepo(scriptRepo), callback(callback) {}

	void OnData(ptr<File> file)
	{
		try
		{
			StreamReader reader(NEW(FileInputStream(file)));
			bool needSync = scriptRepo->clientRepo->ReadWatchResponse(&reader);
			ptr<Script::Np::State> scriptState = callback->GetState();
			callback->Call(
				scriptState->NewBoolean(true),
				scriptState->NewBoolean(needSync));
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
		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewBoolean(false),
			scriptState->NewString(ss.str()));
	}
};

//*** class ScriptRepo

ScriptRepo::ScriptRepo(ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo, ptr<EntityManager> entityManager)
: clientRepo(clientRepo), remoteRepo(remoteRepo), entityManager(entityManager) {}

ptr<EntityManager> ScriptRepo::GetEntityManager() const
{
	return entityManager;
}

void ScriptRepo::Init(ptr<Script::Any> callback)
{
	remoteRepo->GetManifest(NEW(InitHandler(this, callback.FastCast<Script::Np::Any>())));
}

void ScriptRepo::Sync(ptr<Script::Any> callback)
{
	try
	{
		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);
		writer.Write('\n');
		clientRepo->Push(&writer);
		remoteRepo->Sync(stream->ToFile(), NEW(PullHandler(this, callback.FastCast<Script::Np::Any>())));
	}
	catch(Exception* exception)
	{
		try
		{
			clientRepo->Cleanup();
		}
		catch(Exception* exception)
		{
			MakePointer(exception);
		}

		std::ostringstream ss;
		MakePointer(exception)->PrintStack(ss);
		ptr<Script::Np::State> scriptState = callback.FastCast<Script::Np::Any>()->GetState();
		callback->Call(
			scriptState->NewBoolean(false),
			scriptState->NewString(ss.str()));
	}
}

void ScriptRepo::Watch(ptr<Script::Any> callback)
{
	ptr<MemoryStream> stream = NEW(MemoryStream());
	StreamWriter writer(stream);
	writer.Write('\n');
	clientRepo->WriteWatchRequest(&writer);
	remoteRepo->Watch(stream->ToFile(), NEW(WatchHandler(this, callback.FastCast<Script::Np::Any>())));
}

void ScriptRepo::ProcessEvents()
{
	class EventHandler : public ClientRepo::EventHandler
	{
	private:
		ScriptRepo* repo;

	public:
		EventHandler(ScriptRepo* repo) : repo(repo) {}

		void OnEvent(ptr<File> key, ptr<File> value)
		{
			repo->OnChange(key, value);
		}
	};

	clientRepo->ProcessEvents(&EventHandler(this));
}

void ScriptRepo::ApplyAction(ptr<Action> action)
{
	const Action::Changes& changes = action->GetChanges();
	for(size_t i = 0; i < changes.size(); ++i)
	{
		const Action::Change& change = changes[i];
		clientRepo->Change(change.first, change.second);
	}
}

ptr<Action> ScriptRepo::ReverseAction(ptr<Action> action)
{
	ptr<Action> reverseAction = NEW(Action(action->GetDescription()));

	const Action::Changes& changes = action->GetChanges();
	for(size_t i = 0; i < changes.size(); ++i)
	{
		const Action::Change& change = changes[i];
		ptr<File> key = change.first;
		ptr<File> initialValue = clientRepo->GetValue(key);
		reverseAction->AddChange(key, initialValue);
	}

	return reverseAction;
}

void ScriptRepo::FireUndoRedoChangedCallback()
{
	ptr<Action> undoAction = undoActions.empty() ? nullptr : undoActions.back();
	ptr<Action> redoAction = redoActions.empty() ? nullptr : redoActions.back();
	if(undoRedoChangedCallback)
	{
		ptr<Script::Np::State> scriptState = undoRedoChangedCallback->GetState();
		undoRedoChangedCallback->Call(
			scriptState->WrapObject(undoAction),
			scriptState->WrapObject(redoAction));
	}
}

void ScriptRepo::OnChange(ptr<File> key, ptr<File> value)
{
	entityManager->OnChange(key, value);
}

void ScriptRepo::MakeAction(ptr<Action> action)
{
	ptr<Action> undoAction = ReverseAction(action);
	undoActions.push_back(undoAction);

	redoActions.clear();

	ApplyAction(action);

	FireUndoRedoChangedCallback();
}

bool ScriptRepo::Undo()
{
	if(undoActions.empty())
		return false;

	// get undo action
	ptr<Action> undoAction = undoActions.back();
	undoActions.pop_back();

	// create redo action
	ptr<Action> redoAction = ReverseAction(undoAction);

	// apply undo action
	ApplyAction(undoAction);

	// add redo action into redo stack
	redoActions.push_back(redoAction);

	FireUndoRedoChangedCallback();

	return true;
}

bool ScriptRepo::Redo()
{
	if(redoActions.empty())
		return false;

	// get redo action
	ptr<Action> redoAction = redoActions.back();
	redoActions.pop_back();

	// create undo action
	ptr<Action> undoAction = ReverseAction(redoAction);

	// apply redo action
	ApplyAction(redoAction);

	// add undo action into undo stack
	undoActions.push_back(undoAction);

	FireUndoRedoChangedCallback();

	return true;
}

void ScriptRepo::SetUndoRedoChangedCallback(ptr<Script::Any> callback)
{
	this->undoRedoChangedCallback = callback.FastCast<Script::Np::Any>();
}

END_INANITY_OIL

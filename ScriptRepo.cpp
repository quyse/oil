#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "RemoteRepo.hpp"
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
	ptr<Script::Any> callback;

public:
	InitHandler(ptr<ScriptRepo> scriptRepo, ptr<Script::Any> callback)
	: scriptRepo(scriptRepo), callback(callback) {}

	void OnData(ptr<File> data)
	{
		try
		{
			scriptRepo->clientRepo->ReadServerManifest(&StreamReader(NEW(FileInputStream(data))));
			callback->Call(
				scriptRepo->scriptState->NewBoolean(true),
				scriptRepo->scriptState->NewString("OK"));
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
			scriptRepo->scriptState->NewBoolean(false),
			scriptRepo->scriptState->NewString(ss.str()));
	}
};

//*** class ScriptRepo::PullHandler

class ScriptRepo::PullHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptRepo> scriptRepo;
	ptr<Script::Any> callback;

public:
	PullHandler(ptr<ScriptRepo> scriptRepo, ptr<Script::Any> callback)
	: scriptRepo(scriptRepo), callback(callback) {}

	void OnData(ptr<File> data)
	{
		try
		{
			bool changedSomething = scriptRepo->clientRepo->Pull(&StreamReader(NEW(FileInputStream(data))));
			callback->Call(
				scriptRepo->scriptState->NewBoolean(true),
				scriptRepo->scriptState->NewBoolean(changedSomething));
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
			scriptRepo->scriptState->NewBoolean(false),
			scriptRepo->scriptState->NewString(ss.str()));
	}
};

//*** class ScriptRepo::WatchHandler

class ScriptRepo::WatchHandler : public DataHandler<ptr<File> >
{
private:
	ptr<ScriptRepo> scriptRepo;
	ptr<Script::Any> callback;

public:
	WatchHandler(ptr<ScriptRepo> scriptRepo, ptr<Script::Any> callback)
	: scriptRepo(scriptRepo), callback(callback) {}

	void OnData(ptr<File> file)
	{
		try
		{
			StreamReader reader(NEW(FileInputStream(file)));
			bool needSync = scriptRepo->clientRepo->ReadWatchResponse(&reader);
			callback->Call(
				scriptRepo->scriptState->NewBoolean(true),
				scriptRepo->scriptState->NewBoolean(needSync));
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
			scriptRepo->scriptState->NewBoolean(false),
			scriptRepo->scriptState->NewString(ss.str()));
	}
};

//*** class ScriptRepo

ScriptRepo::ScriptRepo(ptr<Script::Np::State> scriptState, ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo)
: scriptState(scriptState), clientRepo(clientRepo), remoteRepo(remoteRepo) {}

void ScriptRepo::Init(ptr<Script::Any> callback)
{
	remoteRepo->GetManifest(NEW(InitHandler(this, callback)));
}

void ScriptRepo::Sync(ptr<Script::Any> callback)
{
	try
	{
		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);
		writer.Write('\n');
		clientRepo->Push(&writer);
		remoteRepo->Sync(stream->ToFile(), NEW(PullHandler(this, callback)));
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
	remoteRepo->Watch(stream->ToFile(), NEW(WatchHandler(this, callback)));
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
		undoRedoChangedCallback->Call(
			scriptState->WrapObject(undoAction),
			scriptState->WrapObject(redoAction));
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

	ptr<Action> undoAction = undoActions.back();
	undoActions.pop_back();

	ApplyAction(undoAction);

	ptr<Action> redoAction = ReverseAction(undoAction);
	redoActions.push_back(redoAction);

	FireUndoRedoChangedCallback();

	return true;
}

bool ScriptRepo::Redo()
{
	if(redoActions.empty())
		return false;

	ptr<Action> redoAction = redoActions.back();
	redoActions.pop_back();

	ApplyAction(redoAction);

	ptr<Action> undoAction = ReverseAction(redoAction);
	undoActions.push_back(undoAction);

	FireUndoRedoChangedCallback();

	return true;
}

void ScriptRepo::SetUndoRedoChangedCallback(ptr<Script::Any> callback)
{
	this->undoRedoChangedCallback = callback;
}

END_INANITY_OIL

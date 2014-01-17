#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "RemoteRepo.hpp"
#include "../inanity/script/State.hpp"
#include "../inanity/script/Any.hpp"
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

ScriptRepo::ScriptRepo(ptr<Script::State> scriptState, ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo)
: scriptState(scriptState), clientRepo(clientRepo), remoteRepo(remoteRepo) {}

void ScriptRepo::Init(ptr<Script::Any> callback)
{
	remoteRepo->GetManifest(NEW(InitHandler(this, callback)));
}

void ScriptRepo::Sync(ptr<Script::Any> callback)
{
	ptr<MemoryStream> stream = NEW(MemoryStream());
	StreamWriter writer(stream);
	clientRepo->Push(&writer);
	remoteRepo->Sync(stream->ToFile(), NEW(PullHandler(this, callback)));
}

void ScriptRepo::Watch(ptr<Script::Any> callback)
{
	ptr<MemoryStream> stream = NEW(MemoryStream());
	StreamWriter writer(stream);
	clientRepo->WriteWatchRequest(&writer);
	remoteRepo->Watch(stream->ToFile(), NEW(WatchHandler(this, callback)));
}

END_INANITY_OIL

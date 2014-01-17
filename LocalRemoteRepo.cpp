#include "LocalRemoteRepo.hpp"
#include "ServerRepo.hpp"
#include "../inanity/MemoryStream.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

LocalRemoteRepo::LocalRemoteRepo(ptr<ServerRepo> serverRepo)
: serverRepo(serverRepo) {}

void LocalRemoteRepo::GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler)
{
	try
	{
		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);
		serverRepo->WriteManifest(&writer);
		manifestHandler->FireData(stream->ToFile());
	}
	catch(Exception* exception)
	{
		manifestHandler->FireError(exception);
	}
}

void LocalRemoteRepo::Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler)
{
	try
	{
		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);
		StreamReader reader(NEW(FileInputStream(pushData)));
		bool pushedSomething = serverRepo->Sync(&reader, &writer);
		pullHandler->FireData(stream->ToFile());

		if(pushedSomething && deferredWatchHandler)
		{
			ptr<MemoryStream> stream = NEW(MemoryStream());
			StreamWriter writer(stream);
			serverRepo->RespondWatch(&writer);
			deferredWatchHandler->FireData(stream->ToFile());
			deferredWatchHandler = nullptr;
		}
	}
	catch(Exception* exception)
	{
		pullHandler->FireError(exception);
	}
}

void LocalRemoteRepo::Watch(ptr<File> requestData, ptr<DataHandler<ptr<File> > > watchHandler)
{
	try
	{
		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);
		StreamReader reader(NEW(FileInputStream(requestData)));
		if(serverRepo->Watch(&reader, &writer))
			watchHandler->FireData(stream->ToFile());
		else
			deferredWatchHandler = watchHandler;
	}
	catch(Exception* exception)
	{
		watchHandler->FireError(exception);
	}
}

END_INANITY_OIL

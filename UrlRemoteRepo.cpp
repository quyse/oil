#include "UrlRemoteRepo.hpp"
#include "MainPluginInstance.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

//*** class UrlRemoteRepo

UrlRemoteRepo::UrlRemoteRepo(const String& url)
: url(url)
{
	syncUrl = url + "?sync";
	watchUrl = url + "?watch";
}

void UrlRemoteRepo::GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler)
{
	MainPluginInstance::GetInstance()->GetUrl(url + "?manifest", manifestHandler);
}

void UrlRemoteRepo::Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler)
{
	MainPluginInstance::GetInstance()->PostUrl(syncUrl, pushData, pullHandler);
}

void UrlRemoteRepo::Watch(ptr<File> requestData, ptr<DataHandler<ptr<File> > > watchHandler)
{
	MainPluginInstance::GetInstance()->PostUrl(watchUrl, requestData, watchHandler);
}

END_INANITY_OIL

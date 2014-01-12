#include "UrlRemoteRepo.hpp"
#include "MainPluginInstance.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

UrlRemoteRepo::UrlRemoteRepo(const String& url)
: url(url)
{
	syncUrl = url + "?sync";
}

void UrlRemoteRepo::GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler)
{
	MainPluginInstance::GetInstance()->GetUrl(url + "?manifest", manifestHandler);
}

void UrlRemoteRepo::Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler)
{
	MainPluginInstance::GetInstance()->PostUrl(syncUrl, pushData, pullHandler);
}

END_INANITY_OIL

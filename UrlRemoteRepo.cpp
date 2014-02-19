#include "UrlRemoteRepo.hpp"
#include "../inanity/platform/NpapiPluginInstance.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/File.hpp"

BEGIN_INANITY_OIL

//*** class UrlRemoteRepo

UrlRemoteRepo::UrlRemoteRepo(Platform::NpapiPluginInstance* pluginInstance, const String& url)
: pluginInstance(pluginInstance), url(url)
{
	syncUrl = url + "?sync";
	watchUrl = url + "?watch";
}

void UrlRemoteRepo::GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler)
{
	pluginInstance->GetUrl(url + "?manifest", manifestHandler);
}

void UrlRemoteRepo::Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler)
{
	pluginInstance->PostUrl(syncUrl, pushData, pullHandler);
}

void UrlRemoteRepo::Watch(ptr<File> requestData, ptr<DataHandler<ptr<File> > > watchHandler)
{
	pluginInstance->PostUrl(watchUrl, requestData, watchHandler);
}

void UrlRemoteRepo::WriteHeader(StreamWriter* writer)
{
	// to circumvent crazy Netscape API we have to prepend POST data with '\n'
	writer->Write('\n');
}

END_INANITY_OIL

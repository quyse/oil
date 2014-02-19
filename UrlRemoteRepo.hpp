#ifndef ___INANITY_OIL_URL_REMOTE_REPO_HPP___
#define ___INANITY_OIL_URL_REMOTE_REPO_HPP___

#include "RemoteRepo.hpp"
#include "../inanity/platform/platform.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY_PLATFORM

class NpapiPluginInstance;

END_INANITY_PLATFORM

BEGIN_INANITY_OIL

/// Remote repo working via url.
class UrlRemoteRepo : public RemoteRepo
{
private:
	Platform::NpapiPluginInstance* pluginInstance;
	String url;
	String syncUrl;
	String watchUrl;

public:
	UrlRemoteRepo(Platform::NpapiPluginInstance* pluginInstance, const String& url);

	//*** RemoteRepo's methods.
	void GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler);
	void Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler);
	void Watch(ptr<File> requestData, ptr<DataHandler<ptr<File> > > watchHandler);
	void WriteHeader(StreamWriter* writer);

	META_DECLARE_CLASS(UrlRemoteRepo);
};

END_INANITY_OIL

#endif

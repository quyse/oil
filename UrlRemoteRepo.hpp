#ifndef ___INANITY_OIL_URL_REMOTE_REPO_HPP___
#define ___INANITY_OIL_URL_REMOTE_REPO_HPP___

#include "RemoteRepo.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY_OIL

/// Remote repo working via url.
class UrlRemoteRepo : public RemoteRepo
{
private:
	String url;
	String syncUrl;

public:
	UrlRemoteRepo(const String& url);

	//*** RemoteRepo's methods.
	void GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler);
	void Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler);
};

END_INANITY_OIL

#endif

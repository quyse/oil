#ifndef ___INANITY_OIL_REMOTE_REPO_HPP___
#define ___INANITY_OIL_REMOTE_REPO_HPP___

#include "oil.hpp"
#include "../inanity/Handler.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_OIL

/// Abstract remote repo.
/** Provides link with server repo from client side. */
class RemoteRepo : public Object
{
public:
	virtual void GetManifest(ptr<DataHandler<ptr<File> > > manifestHandler) = 0;
	virtual void Sync(ptr<File> pushData, ptr<DataHandler<ptr<File> > > pullHandler) = 0;
	virtual void Watch(ptr<File> requestData, ptr<DataHandler<ptr<File> > > watchHandler) = 0;
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_SCRIPT_REPO_HPP___
#define ___INANITY_OIL_SCRIPT_REPO_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class ClientRepo;
class RemoteRepo;

class ScriptRepo : public Object
{
private:
	ptr<ClientRepo> clientRepo;
	ptr<RemoteRepo> remoteRepo;

public:
	ScriptRepo(ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo);
};

END_INANITY_OIL

#endif

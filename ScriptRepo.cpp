#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"
#include "RemoteRepo.hpp"

BEGIN_INANITY_OIL

ScriptRepo::ScriptRepo(ptr<ClientRepo> clientRepo, ptr<RemoteRepo> remoteRepo)
: clientRepo(clientRepo), remoteRepo(remoteRepo)
{
}

END_INANITY_OIL

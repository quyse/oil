#include "ScriptRepo.hpp"
#include "ClientRepo.hpp"

BEGIN_INANITY_OIL

ScriptRepo::ScriptRepo(ptr<ClientRepo> repo, const String& serverUrl)
: repo(repo), serverUrl(serverUrl)
{
}

END_INANITY_OIL

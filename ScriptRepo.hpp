#ifndef ___INANITY_OIL_SCRIPT_REPO_HPP___
#define ___INANITY_OIL_SCRIPT_REPO_HPP___

#include "oil.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY_OIL

class ClientRepo;

class ScriptRepo : public Object
{
private:
	ptr<ClientRepo> repo;
	String serverUrl;

public:
	ScriptRepo(ptr<ClientRepo> repo, const String& serverUrl);
};

END_INANITY_OIL

#endif

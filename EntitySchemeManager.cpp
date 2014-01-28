#include "EntitySchemeManager.hpp"
#include "EntityScheme.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

ptr<EntityScheme> EntitySchemeManager::TryGet(const EntitySchemeId& schemeId) const
{
	Schemes::const_iterator i = schemes.find(schemeId);
	if(i == schemes.end())
		return nullptr;
	return i->second;
}

ptr<EntityScheme> EntitySchemeManager::Get(const EntitySchemeId& schemeId) const
{
	ptr<EntityScheme> scheme = TryGet(schemeId);
	if(!scheme)
		THROW("There is no such scheme");
	return scheme;
}

void EntitySchemeManager::Register(ptr<EntityScheme> scheme)
{
	EntitySchemeId schemeId = scheme->GetId();

	if(schemes.find(schemeId) != schemes.end())
		THROW("Scheme already registered");

	schemes.insert(std::make_pair(schemeId, scheme));
}

END_INANITY_OIL

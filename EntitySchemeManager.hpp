#ifndef ___INANITY_OIL_ENTITY_SCHEME_MANAGER_HPP___
#define ___INANITY_OIL_ENTITY_SCHEME_MANAGER_HPP___

#include "Id.hpp"
#include <map>

BEGIN_INANITY_OIL

class EntityScheme;

/// Manager of entity schemes.
class EntitySchemeManager : public Object
{
private:
	typedef std::map<EntitySchemeId, ptr<EntityScheme> > Schemes;
	Schemes schemes;

public:
	/// Get a scheme by id.
	ptr<EntityScheme> TryGet(const EntitySchemeId& schemeId) const;
	ptr<EntityScheme> Get(const EntitySchemeId& schemeId) const;
	/// Register scheme into manager.
	void Register(ptr<EntityScheme> scheme);
};

END_INANITY_OIL

#endif

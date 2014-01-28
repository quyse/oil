#ifndef ___INANITY_OIL_ENTITY_SCHEME_HPP___
#define ___INANITY_OIL_ENTITY_SCHEME_HPP___

#include "EntityId.hpp"

BEGIN_INANITY_OIL

class ClientRepo;
class EntityData;

class EntityScheme : public Object
{
protected:
	EntitySchemeId schemeId;

public:
	EntityScheme(const EntitySchemeId& schemeId);

	EntitySchemeId GetId() const;

	/// Create an entity data.
	virtual ptr<EntityData> CreateData(ClientRepo* repo, const EntityId& entityId) = 0;
};

END_INANITY_OIL

#endif

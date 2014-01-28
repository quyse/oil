#include "EntityScheme.hpp"

BEGIN_INANITY_OIL

EntityScheme::EntityScheme(const EntitySchemeId& schemeId)
: schemeId(schemeId) {}

EntitySchemeId EntityScheme::GetId() const
{
	return schemeId;
}

END_INANITY_OIL

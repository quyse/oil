#include "EntityScheme.hpp"

BEGIN_INANITY_OIL

EntityScheme::EntityScheme(const EntitySchemeId& id)
: id(id) {}

EntitySchemeId EntityScheme::GetId() const
{
	return id;
}

const EntityScheme::Fields& EntityScheme::GetFields() const
{
	return fields;
}

END_INANITY_OIL

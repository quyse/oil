#include "EntityData.hpp"
#include "EntityScheme.hpp"

BEGIN_INANITY_OIL

EntityData::EntityData(ptr<EntityScheme> scheme)
: scheme(scheme) {}

ptr<EntityScheme> EntityData::GetScheme() const
{
	return scheme;
}

END_INANITY_OIL

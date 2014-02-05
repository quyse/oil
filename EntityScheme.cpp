#include "EntityScheme.hpp"
#include "EntityFieldType.hpp"

BEGIN_INANITY_OIL

EntityScheme::EntityScheme(const EntitySchemeId& id, const String& name)
: id(id), name(name) {}

EntitySchemeId EntityScheme::GetId() const
{
	return id;
}

String EntityScheme::GetName() const
{
	return name;
}

const EntityScheme::Fields& EntityScheme::GetFields() const
{
	return fields;
}

int EntityScheme::GetFieldsCount() const
{
	return (int)fields.size();
}

const char* EntityScheme::GetFieldType(int fieldIndex) const
{
	return fields[fieldIndex].type->GetName();
}

String EntityScheme::GetFieldName(int fieldIndex) const
{
	return fields[fieldIndex].name;
}

END_INANITY_OIL

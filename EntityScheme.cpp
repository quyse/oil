#include "EntityScheme.hpp"
#include "EntityFieldType.hpp"
#include "../inanity/Exception.hpp"

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

const char* EntityScheme::GetFieldType(const String& fieldId) const
{
	Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		THROW("No such field");
	return i->second.type->GetName();
}

String EntityScheme::GetFieldName(const String& fieldId) const
{
	Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		THROW("No such field");
	return i->second.name;
}

void EntityScheme::AddField(const String& fieldId, const String& type, const String& name)
{
	Field field;
	field.type = EntityFieldType::FromName(type.c_str());
	field.name = name;
	fields[fieldId] = field;
}

END_INANITY_OIL

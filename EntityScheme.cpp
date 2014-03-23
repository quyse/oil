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

EntityFieldId EntityScheme::GetFieldId(int index) const
{
	return fieldIds[index];
}

const char* EntityScheme::GetFieldType(const EntityFieldId& fieldId) const
{
	Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		THROW("No such field");
	return i->second.type->GetName();
}

String EntityScheme::GetFieldName(const EntityFieldId& fieldId) const
{
	Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		THROW("No such field");
	return i->second.name;
}

void EntityScheme::AddField(const EntityFieldId& fieldId, const String& type, const String& name)
{
	Field field;
	field.type = EntityFieldType::FromName(type.c_str());
	field.name = name;
	fields[fieldId] = field;

	fieldIds.push_back(fieldId);
}

END_INANITY_OIL

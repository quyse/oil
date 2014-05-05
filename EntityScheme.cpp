#include "EntityScheme.hpp"
#include "EntityFieldType.hpp"
#include "../inanity/script/Any.hpp"
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

const EntityScheme::Interfaces& EntityScheme::GetInterfaces() const
{
	return interfaces;
}

int EntityScheme::GetFieldsCount() const
{
	return (int)fields.size();
}

EntityFieldId EntityScheme::GetFieldId(int index) const
{
	return fieldIds[index];
}

ptr<EntityFieldType> EntityScheme::GetFieldType(const EntityFieldId& fieldId) const
{
	Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		THROW("No such field");
	return i->second.type;
}

String EntityScheme::GetFieldName(const EntityFieldId& fieldId) const
{
	Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		THROW("No such field");
	return i->second.name;
}

void EntityScheme::AddField(const EntityFieldId& fieldId, ptr<EntityFieldType> type, const String& name)
{
	Field field;
	field.type = type;
	field.name = name;
	fields[fieldId] = field;

	fieldIds.push_back(fieldId);
}

int EntityScheme::GetInterfacesCount() const
{
	return (int)interfaces.size();
}

EntityInterfaceId EntityScheme::GetInterfaceId(int index) const
{
	return interfaceIds[index];
}

bool EntityScheme::HasInterface(const EntityInterfaceId& interfaceId) const
{
	return interfaces.find(interfaceId) != interfaces.end();
}

void EntityScheme::AddInterface(const EntityInterfaceId& interfaceId, ptr<Script::Any> callback)
{
	Interface interf;
	interf.callback = callback;
	interfaces[interfaceId] = interf;

	interfaceIds.push_back(interfaceId);
}

END_INANITY_OIL

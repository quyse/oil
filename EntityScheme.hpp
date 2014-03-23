#ifndef ___INANITY_OIL_ENTITY_SCHEME_HPP___
#define ___INANITY_OIL_ENTITY_SCHEME_HPP___

#include "Id.hpp"
#include <map>
#include <vector>

BEGIN_INANITY_OIL

class Entity;
class EntityFieldType;

/// Scheme of fields in entity.
class EntityScheme : public Object
{
public:
	struct Field
	{
		EntityFieldType* type;
		String name;
	};
	typedef std::map<EntityFieldId, Field> Fields;

private:
	EntitySchemeId id;
	String name;

	/// Fields of scheme.
	Fields fields;
	/// Fields of scheme by index.
	std::vector<EntityFieldId> fieldIds;

public:
	EntityScheme(const EntitySchemeId& id, const String& name);

	EntitySchemeId GetId() const;
	String GetName() const;

	const Fields& GetFields() const;

	//** for scripts
	int GetFieldsCount() const;
	EntityFieldId GetFieldId(int index) const;
	const char* GetFieldType(const EntityFieldId& fieldId) const;
	String GetFieldName(const EntityFieldId& fieldId) const;
	void AddField(const EntityFieldId& fieldId, const String& type, const String& name);
};

END_INANITY_OIL

#endif

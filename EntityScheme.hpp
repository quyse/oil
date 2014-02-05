#ifndef ___INANITY_OIL_ENTITY_SCHEME_HPP___
#define ___INANITY_OIL_ENTITY_SCHEME_HPP___

#include "EntityId.hpp"
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
	typedef std::vector<Field> Fields;

private:
	EntitySchemeId id;
	String name;

	/// Fields of scheme.
	Fields fields;

public:
	EntityScheme(const EntitySchemeId& id, const String& name);

	EntitySchemeId GetId() const;
	String GetName() const;

	const Fields& GetFields() const;

	//** for scripts
	int GetFieldsCount() const;
	const char* GetFieldType(int fieldIndex) const;
	String GetFieldName(int fieldIndex) const;
};

END_INANITY_OIL

#endif

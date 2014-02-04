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

	/// Fields of scheme.
	Fields fields;

public:
	EntityScheme(const EntitySchemeId& id);

	EntitySchemeId GetId() const;

	const Fields& GetFields() const;
};

END_INANITY_OIL

#endif

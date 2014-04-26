#ifndef ___INANITY_OIL_ENTITY_SCHEME_MANAGER_HPP___
#define ___INANITY_OIL_ENTITY_SCHEME_MANAGER_HPP___

#include "Id.hpp"
#include <map>

BEGIN_INANITY_OIL

class EntityScheme;
class EntityFieldType;

/// Manager of entity schemes.
class EntitySchemeManager : public Object
{
private:
	typedef std::map<EntitySchemeId, ptr<EntityScheme> > Schemes;
	Schemes schemes;

	/// Standard field types.
	ptr<EntityFieldType>
		blobFieldType,
		boolFieldType,
		floatFieldType,
		integerFieldType,
		stringFieldType,
		vec3FieldType,
		vec4FieldType,
		color3FieldType,
		color4FieldType;

public:
	EntitySchemeManager();

	/// Get a scheme by id.
	ptr<EntityScheme> TryGetScheme(const EntitySchemeId& schemeId) const;
	ptr<EntityScheme> GetScheme(const EntitySchemeId& schemeId) const;
	/// Register scheme into manager.
	void RegisterScheme(ptr<EntityScheme> scheme);

	/// Get standard field type.
	ptr<EntityFieldType> GetStandardFieldType(const String& name) const;
};

END_INANITY_OIL

#endif

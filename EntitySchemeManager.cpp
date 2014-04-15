#include "EntitySchemeManager.hpp"
#include "EntityScheme.hpp"
#include "EntityFieldTypes.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

EntitySchemeManager::EntitySchemeManager()
{
	// init standard field types
	blobFieldType = NEW(BlobEntityFieldType());
	boolFieldType = NEW(BoolEntityFieldType());
	floatFieldType = NEW(FloatEntityFieldType());
	integerFieldType = NEW(IntegerEntityFieldType());
	stringFieldType = NEW(StringEntityFieldType());
	vec3FieldType = NEW(Vec3EntityFieldType());
	vec4FieldType = NEW(Vec4EntityFieldType());
	color3FieldType = NEW(Color3EntityFieldType());
	color4FieldType = NEW(Color4EntityFieldType());
}

ptr<EntityScheme> EntitySchemeManager::TryGetScheme(const EntitySchemeId& schemeId) const
{
	Schemes::const_iterator i = schemes.find(schemeId);
	if(i == schemes.end())
		return nullptr;
	return i->second;
}

ptr<EntityScheme> EntitySchemeManager::GetScheme(const EntitySchemeId& schemeId) const
{
	ptr<EntityScheme> scheme = TryGetScheme(schemeId);
	if(!scheme)
		THROW("There is no such scheme");
	return scheme;
}

void EntitySchemeManager::RegisterScheme(ptr<EntityScheme> scheme)
{
	EntitySchemeId schemeId = scheme->GetId();

	if(schemes.find(schemeId) != schemes.end())
		THROW("Scheme already registered");

	schemes.insert(std::make_pair(schemeId, scheme));
}

ptr<EntityFieldType> EntitySchemeManager::GetStandardFieldType(const String& name) const
{
#define T(type) \
	if(type##FieldType->GetName() == name) \
		return type##FieldType

	T(blob);
	T(bool);
	T(float);
	T(integer);
	T(string);
	T(vec3);
	T(vec4);
	T(color3);
	T(color4);
#undef T

	return nullptr;
}

END_INANITY_OIL

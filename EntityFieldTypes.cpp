#include "EntityFieldTypes.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/script/convert.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/Strings.hpp"
#include "../inanity/inanity-math.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_SCRIPT

extern template Math::vec3 ConvertFromScript<Math::vec3>(ptr<Any> value);
extern template Math::vec4 ConvertFromScript<Math::vec4>(ptr<Any> value);
extern template ptr<Any> ConvertToScript<Math::vec3>(ptr<State> state, const Math::vec3& value);
extern template ptr<Any> ConvertToScript<Math::vec4>(ptr<State> state, const Math::vec4& value);

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

//*** class FloatEntityFieldType

const char* FloatEntityFieldType::GetName() const
{
	return "float";
}

ptr<Script::Any> FloatEntityFieldType::TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value)
{
	if(value->GetSize() != sizeof(float))
		return nullptr;
	return scriptState->NewNumber(*(const float*)value->GetData());
}

ptr<File> FloatEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	float number = (float)value->AsFloat();
	return MemoryFile::CreateViaCopy(&number, sizeof(number));
}

//*** class IntegerEntityFieldType

const char* IntegerEntityFieldType::GetName() const
{
	return "integer";
}

ptr<Script::Any> IntegerEntityFieldType::TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value)
{
	if(value->GetSize() != sizeof(int32_t))
		return nullptr;
	return scriptState->NewNumber(*(const int32_t*)value->GetData());
}

ptr<File> IntegerEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	int32_t number = (int32_t)value->AsInt();
	return MemoryFile::CreateViaCopy(&number, sizeof(number));
}

//*** class StringEntityFieldType

const char* StringEntityFieldType::GetName() const
{
	return "string";
}

ptr<Script::Any> StringEntityFieldType::TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return scriptState->NewString(Strings::File2String(value));
}

ptr<File> StringEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	String string = value->AsString();
	return MemoryFile::CreateViaCopy(string.c_str(), string.length());
}

//*** class Vec3EntityFieldType

const char* Vec3EntityFieldType::GetName() const
{
	return "vec3";
}

ptr<Script::Any> Vec3EntityFieldType::TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value)
{
	if(value->GetSize() != sizeof(Math::vec3))
		return nullptr;
	return Script::ConvertToScript(scriptState, *(const Math::vec3*)value->GetData());
}

ptr<File> Vec3EntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	Math::vec3 v;
	try
	{
		v = Script::ConvertFromScript<Math::vec3>(value);
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
		return nullptr;
	}
	return MemoryFile::CreateViaCopy(&v, sizeof(v));
}

//*** class Vec4EntityFieldType

const char* Vec4EntityFieldType::GetName() const
{
	return "vec4";
}

ptr<Script::Any> Vec4EntityFieldType::TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value)
{
	if(value->GetSize() != sizeof(Math::vec4))
		return nullptr;
	return Script::ConvertToScript(scriptState, *(const Math::vec4*)value->GetData());
}

ptr<File> Vec4EntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	Math::vec4 v;
	try
	{
		v = Script::ConvertFromScript<Math::vec4>(value);
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
		return nullptr;
	}
	return MemoryFile::CreateViaCopy(&v, sizeof(v));
}

//*** class Color3EntityFieldType

const char* Color3EntityFieldType::GetName() const
{
	return "color3";
}

//*** class Color4EntityFieldType

const char* Color4EntityFieldType::GetName() const
{
	return "color4";
}

//*** class ReferenceEntityFieldType

const char* ReferenceEntityFieldType::GetName() const
{
	return "reference";
}

ptr<Script::Any> ReferenceEntityFieldType::TryConvertToScript(ptr<Script::Np::State> scriptState, ptr<File> value)
{
	try
	{
		EntityId entityId = EntityId::FromString(Strings::File2String(value).c_str());
		return scriptState->WrapObject(entityManager->GetEntity(entityId));
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
		return nullptr;
	}
}

ptr<File> ReferenceEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	ptr<Entity> entity = value->AsObject().DynamicCast<Entity>();
	if(!entity)
		return nullptr;
	String string = entity->GetId().ToString();
	return MemoryFile::CreateViaCopy(string.c_str(), string.length());
}

END_INANITY_OIL

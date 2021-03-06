#include "EntityFieldTypes.hpp"
#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityManager.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/script/convert.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/Strings.hpp"
#include "../inanity/inanity-math.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_SCRIPT

extern template Math::vec3 ConvertFromScript<Np::MetaProvider, Math::vec3>(ptr<Any> value);
extern template Math::vec4 ConvertFromScript<Np::MetaProvider, Math::vec4>(ptr<Any> value);
extern template ptr<Any> ConvertToScript<Np::MetaProvider, Math::vec3>(ptr<State> state, const Math::vec3& value);
extern template ptr<Any> ConvertToScript<Np::MetaProvider, Math::vec4>(ptr<State> state, const Math::vec4& value);

END_INANITY_SCRIPT

BEGIN_INANITY_OIL

//*** class BlobEntityFieldType

String BlobEntityFieldType::GetName() const
{
	return "blob";
}

ptr<Script::Any> BlobEntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return scriptState->WrapObject(value);
}

ptr<File> BlobEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	return value->AsObject().DynamicCast<File>();
}

//*** class BoolEntityFieldType

String BoolEntityFieldType::GetName() const
{
	return "bool";
}

ptr<Script::Any> BoolEntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	bool allZeroes = true;
	if(value)
	{
		const char* valueData = (const char*)value->GetData();
		size_t valueSize = value->GetSize();
		for(size_t i = 0; i < valueSize; ++i)
			if(valueData[i])
			{
				allZeroes = false;
				break;
			}
	}

	return scriptState->NewBoolean(!allZeroes);
}

ptr<File> BoolEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	ptr<MemoryFile> file = NEW(MemoryFile(1));
	*(char*)file->GetData() = value->AsBool() ? 1 : 0;
	return file;
}

//*** class FloatEntityFieldType

String FloatEntityFieldType::GetName() const
{
	return "float";
}

ptr<Script::Any> FloatEntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return scriptState->NewNumber(value && value->GetSize() == sizeof(float) ? *(const float*)value->GetData() : 0.0f);
}

ptr<File> FloatEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	float number = (float)value->AsFloat();
	return MemoryFile::CreateViaCopy(&number, sizeof(number));
}

//*** class IntegerEntityFieldType

String IntegerEntityFieldType::GetName() const
{
	return "integer";
}

ptr<Script::Any> IntegerEntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return scriptState->NewNumber(value && value->GetSize() == sizeof(int32_t) ? *(const int32_t*)value->GetData() : 0);
}

ptr<File> IntegerEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	int32_t number = (int32_t)value->AsInt();
	return MemoryFile::CreateViaCopy(&number, sizeof(number));
}

//*** class StringEntityFieldType

String StringEntityFieldType::GetName() const
{
	return "string";
}

ptr<Script::Any> StringEntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return scriptState->NewString(value ? Strings::File2String(value) : String());
}

ptr<File> StringEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	String string = value->AsString();
	return MemoryFile::CreateViaCopy(string.c_str(), string.length());
}

//*** class Vec3EntityFieldType

String Vec3EntityFieldType::GetName() const
{
	return "vec3";
}

ptr<Script::Any> Vec3EntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return Script::ConvertToScript<Script::Np::MetaProvider, Math::vec3>(scriptState, value && value->GetSize() == sizeof(Math::vec3) ? *(const Math::vec3*)value->GetData() : Math::vec3(0, 0, 0));
}

ptr<File> Vec3EntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	Math::vec3 v;
	try
	{
		v = Script::ConvertFromScript<Script::Np::MetaProvider, Math::vec3>(value);
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
		return nullptr;
	}
	return MemoryFile::CreateViaCopy(&v, sizeof(v));
}

//*** class Vec4EntityFieldType

String Vec4EntityFieldType::GetName() const
{
	return "vec4";
}

ptr<Script::Any> Vec4EntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	return Script::ConvertToScript<Script::Np::MetaProvider, Math::vec4>(scriptState, value && value->GetSize() == sizeof(Math::vec4) ? *(const Math::vec4*)value->GetData() : Math::vec4(0, 0, 0, 0));
}

ptr<File> Vec4EntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	Math::vec4 v;
	try
	{
		v = Script::ConvertFromScript<Script::Np::MetaProvider, Math::vec4>(value);
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
		return nullptr;
	}
	return MemoryFile::CreateViaCopy(&v, sizeof(v));
}

//*** class Color3EntityFieldType

String Color3EntityFieldType::GetName() const
{
	return "color3";
}

//*** class Color4EntityFieldType

String Color4EntityFieldType::GetName() const
{
	return "color4";
}

//*** class ReferenceEntityFieldType

String ReferenceEntityFieldType::GetName() const
{
	return "reference";
}

ptr<Script::Any> ReferenceEntityFieldType::TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value)
{
	try
	{
		if(value)
			return scriptState->NewString(EntityId::FromFile(value).ToString());
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
	}
	return scriptState->NewString("");
}

ptr<File> ReferenceEntityFieldType::TryConvertFromScript(ptr<Script::Np::Any> value)
{
	try
	{
		return EntityId::FromString(value->AsString()).ToFile();
	}
	catch(Exception* exception)
	{
		MakePointer(exception);
		return nullptr;
	}
}

END_INANITY_OIL

#include "EntityCallback.hpp"
#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityFieldType.hpp"
#include "../inanity/File.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/Log.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

EntityCallback::EntityCallback(ptr<Entity> entity, ptr<Script::Np::Any> callback)
: entity(entity), callback(callback)
{
	entity->OnNewCallback(this);
}

EntityCallback::~EntityCallback()
{
	entity->OnFreeCallback(this);
}

void EntityCallback::FireScheme(ptr<EntityScheme> scheme)
{
	try
	{
		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewString("scheme"),
			scriptState->WrapObject<RefCounted>(nullptr),
			scriptState->WrapObject(scheme));
	}
	catch(Exception* exception)
	{
		Log::Error(exception);
	}
}

void EntityCallback::FireTag(const EntityTagId& tagId, ptr<File> value)
{
	try
	{
		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewString("tag"),
			scriptState->NewString(tagId.ToString()),
			scriptState->WrapObject(value));
	}
	catch(Exception* exception)
	{
		Log::Error(exception);
	}
}

void EntityCallback::FireField(const String& fieldId, ptr<File> value)
{
	try
	{
		const EntityScheme::Fields& fields = entity->GetScheme()->GetFields();
		EntityScheme::Fields::const_iterator i = fields.find(fieldId);
		if(i == fields.end())
			return;

		ptr<Script::Any> scriptValue = i->second.type->
			TryConvertToScript(entity->GetManager(), callback->GetState(), value);

		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewString("field"),
			scriptState->NewString(fieldId),
			scriptValue);
	}
	catch(Exception* exception)
	{
		Log::Error(exception);
	}
}

void EntityCallback::FireData(ptr<File> key, ptr<File> value)
{
	try
	{
		ptr<Script::Np::State> scriptState = callback->GetState();
		callback->Call(
			scriptState->NewString("data"),
			scriptState->WrapObject(key),
			scriptState->WrapObject(value));
	}
	catch(Exception* exception)
	{
		Log::Error(exception);
	}
}

void EntityCallback::EnumerateFields()
{
	class Enumerator : public Entity::FieldEnumerator
	{
	private:
		EntityCallback* callback;

	public:
		Enumerator(EntityCallback* callback)
		: callback(callback) {}

		void OnField(const String& fieldId, ptr<File> value)
		{
			callback->FireField(fieldId, value);
		}
	};

	entity->EnumerateFields(&Enumerator(this));
}

void EntityCallback::EnumerateData()
{
	class Enumerator : public Entity::DataEnumerator
	{
	private:
		EntityCallback* callback;

	public:
		Enumerator(EntityCallback* callback)
		: callback(callback) {}

		void OnData(ptr<File> key, ptr<File> value)
		{
			callback->FireData(key, value);
		}
	};

	entity->EnumerateData(&Enumerator(this));
}

END_INANITY_OIL

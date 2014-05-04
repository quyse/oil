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

void EntityCallback::FireField(const EntityFieldId& fieldId, ptr<File> value)
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
			scriptState->NewString(fieldId.ToString()),
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

void EntityCallback::EnumerateScheme()
{
	FireScheme(entity->GetScheme());
}

void EntityCallback::EnumerateTag(const EntityTagId& tagId)
{
	FireTag(tagId, entity->ReadTag(tagId));
}

void EntityCallback::EnumerateFields()
{
	class Enumerator : public Entity::FieldEnumerator
	{
	private:
		std::vector<std::pair<EntityFieldId, ptr<File> > > fields;

	public:
		void OnField(const EntityFieldId& fieldId, ptr<File> value)
		{
			fields.push_back(std::make_pair(fieldId, value));
		}

		void Fire(EntityCallback* callback)
		{
			for(size_t i = 0; i < fields.size(); ++i)
				callback->FireField(fields[i].first, fields[i].second);
		}
	};

	Enumerator enumerator;
	entity->EnumerateFields(&enumerator);
	enumerator.Fire(this);
}

void EntityCallback::EnumerateData()
{
	class Enumerator : public Entity::DataEnumerator
	{
	private:
		std::vector<std::pair<ptr<File>, ptr<File> > > data;

	public:
		void OnData(ptr<File> key, ptr<File> value)
		{
			data.push_back(std::make_pair(key, value));
		}

		void Fire(EntityCallback* callback)
		{
			for(size_t i = 0; i < data.size(); ++i)
				callback->FireData(data[i].first, data[i].second);
		}
	};

	Enumerator enumerator;
	entity->EnumerateData(&enumerator);
	enumerator.Fire(this);
}

END_INANITY_OIL

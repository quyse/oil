#include "EntityCallback.hpp"
#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityFieldType.hpp"
#include "../inanity/File.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/script/np/State.hpp"

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
	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("scheme"),
		scriptState->WrapObject<RefCounted>(nullptr),
		scriptState->WrapObject(scheme));
}

void EntityCallback::FireTag(const EntityTagId& tagId, ptr<File> value)
{
	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("tag"),
		scriptState->NewString(tagId.ToString()),
		scriptState->WrapObject(value));
}

void EntityCallback::FireField(int fieldIndex, ptr<File> value)
{
	const EntityScheme::Fields& fields = entity->GetScheme()->GetFields();
	if(fieldIndex >= (int)fields.size())
		return;

	ptr<Script::Any> scriptValue = fields[fieldIndex].type->TryConvertToScript(callback->GetState(), value);

	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("field"),
		scriptState->NewNumber(fieldIndex),
		scriptValue);
}

void EntityCallback::FireData(ptr<File> key, ptr<File> value)
{
	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("data"),
		scriptState->WrapObject(key),
		scriptState->WrapObject(value));
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

		void OnField(int fieldIndex, ptr<File> value)
		{
			callback->FireField(fieldIndex, value);
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

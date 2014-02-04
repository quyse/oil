#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityManager.hpp"
#include "EntityFieldType.hpp"
#include "Action.hpp"
#include "ClientRepo.hpp"
#include "../inanity/script/np/Any.hpp"
#include "../inanity/script/np/State.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"

BEGIN_INANITY_OIL

Entity::Entity(ptr<EntityManager> manager, const EntityId& id)
: manager(manager), id(id)
{
	manager->OnNewEntity(id, this);
}

Entity::~Entity()
{
	manager->OnFreeEntity(id);
}

ptr<EntityManager> Entity::GetManager() const
{
	return manager;
}

EntityId Entity::GetId() const
{
	return id;
}

ptr<EntityScheme> Entity::GetScheme() const
{
	return scheme;
}

void Entity::SetScheme(ptr<EntityScheme> scheme)
{
	this->scheme = scheme;
}

ptr<File> Entity::GetFullTagKey(const EntityTagId& tagId) const
{
	ptr<MemoryFile> key = NEW(MemoryFile(EntityId::size + 1 + EntityTagId::size));
	char* keyData = (char*)key->GetData();
	memcpy(keyData, id.data, EntityId::size);
	keyData[EntityId::size] = 't';
	memcpy(keyData + EntityId::size + 1, tagId.data, EntityTagId::size);
	return key;
}

ptr<File> Entity::GetFullFieldKey(int fieldIndex) const
{
	if(fieldIndex >= (1 << 16))
		THROW("Too big field index");

	ptr<MemoryFile> key = NEW(MemoryFile(EntityId::size + 1 + 2));
	char* keyData = (char*)key->GetData();
	memcpy(keyData, id.data, EntityId::size);
	keyData[EntityId::size] = 'f';
	keyData[EntityId::size + 1] = fieldIndex & 0xff;
	keyData[EntityId::size + 2] = (fieldIndex >> 8) & 0xff;

	return key;
}

int Entity::TryParseFieldKey(const void* data, size_t size)
{
	if(size != 2)
		return -1;

	const unsigned char* d = (const unsigned char*)data;
	return d[0] | (d[1] << 8);
}

void Entity::FireTagCallback(const EntityTagId& tagId, ptr<File> value)
{
	if(!callback)
		return;

	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("tag"),
		scriptState->NewString(tagId.ToString()),
		scriptState->WrapObject(value));
}

void Entity::FireFieldCallback(int fieldIndex, ptr<File> value)
{
	if(!callback)
		return;

	const EntityScheme::Fields& fields = scheme->GetFields();
	if(fieldIndex >= (int)fields.size())
		return;

	ptr<Script::Any> scriptValue = fields[fieldIndex].type->TryConvertToScript(callback->GetState(), value);

	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("field"),
		scriptState->NewNumber(fieldIndex),
		scriptValue);
}

void Entity::FireDataCallback(ptr<File> key, ptr<File> value)
{
	if(!callback)
		return;

	ptr<Script::Np::State> scriptState = callback->GetState();
	callback->Call(
		scriptState->NewString("data"),
		scriptState->WrapObject(key),
		scriptState->WrapObject(value));
}

void Entity::OnChange(const void* keyData, size_t keySize, ptr<File> value)
{
	if(!scheme)
		return;

	switch(*(const char*)keyData)
	{
	case 't':
		{
			if(keySize != EntityTagId::size + 1)
				break;
			EntityTagId tagId = EntityTagId::FromData((const char*)keyData + 1);
			FireTagCallback(tagId, value);
		}
		break;
	case 'f':
		{
			int fieldIndex = TryParseFieldKey((const char*)keyData + 1, keySize - 1);
			if(fieldIndex < 0)
				break;
			FireFieldCallback(fieldIndex, value);
		}
		break;
	case 'd':
		FireDataCallback(MemoryFile::CreateViaCopy((const char*)keyData + 1, keySize - 1), value);
		break;
	}
}

ptr<File> Entity::ReadTag(const EntityTagId& tagId) const
{
	return manager->GetRepo()->GetValue(GetFullTagKey(tagId));
}

void Entity::WriteTag(ptr<Action> action, const EntityTagId& tagId, ptr<File> tagData)
{
	action->AddChange(GetFullTagKey(tagId), tagData);
}

ptr<File> Entity::ReadField(size_t fieldIndex) const
{
	return manager->GetRepo()->GetValue(GetFullFieldKey(fieldIndex));
}

void Entity::WriteField(ptr<Action> action, size_t fieldIndex, ptr<File> data)
{
	action->AddChange(GetFullFieldKey(fieldIndex), data);
}

void Entity::EnumerateFields(FieldEnumerator* enumerator)
{
	class Enumerator : public ClientRepo::KeyValueEnumerator
	{
	private:
		FieldEnumerator* enumerator;

	public:
		Enumerator(FieldEnumerator* enumerator)
		: enumerator(enumerator) {}

		bool OnKeyValue(ptr<File> key, ptr<File> value)
		{
			int fieldIndex = TryParseFieldKey(
				(char*)key->GetData() + EntityId::size + 1,
				key->GetSize() - EntityId::size - 1);

			if(fieldIndex < 0)
				return true;

			enumerator->OnField(fieldIndex, value->GetData(), value->GetSize());
			return true;
		}
	};

	ptr<MemoryFile> prefix = NEW(MemoryFile(EntityId::size + 1));
	char* prefixData = (char*)prefix->GetData();
	memcpy(prefixData, id.data, EntityId::size);
	prefixData[EntityId::size] = 'f';

	manager->GetRepo()->EnumerateKeyValues(prefix, &Enumerator(enumerator));
}

void Entity::SetCallback(ptr<Script::Any> callback)
{
	this->callback = callback.FastCast<Script::Np::Any>();
}

END_INANITY_OIL

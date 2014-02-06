#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityManager.hpp"
#include "EntitySchemeManager.hpp"
#include "EntityCallback.hpp"
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

void Entity::OnNewCallback(EntityCallback* callback)
{
	callbacks.push_back(callback);
}

void Entity::OnFreeCallback(EntityCallback* callback)
{
	for(size_t i = 0; i < callbacks.size(); ++i)
		if(callbacks[i] == callback)
		{
			callbacks.erase(callbacks.begin() + i);
			return;
		}

	THROW("Entity callback already freed");
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

ptr<File> Entity::GetFullDataKey(const void* nameData, size_t nameSize) const
{
	ptr<MemoryFile> key = NEW(MemoryFile(EntityId::size + 1 + nameSize));
	char* keyData = (char*)key->GetData();
	memcpy(keyData, id.data, EntityId::size);
	keyData[EntityId::size] = 'd';
	memcpy(keyData + EntityId::size + 1, nameData, nameSize);

	return key;
}

int Entity::TryParseFieldKey(const void* data, size_t size)
{
	if(size != 2)
		return -1;

	const unsigned char* d = (const unsigned char*)data;
	return d[0] | (d[1] << 8);
}

void Entity::OnChange(const void* keyData, size_t keySize, ptr<File> value)
{
	if(!scheme)
		return;

	// if this is a main entity key (scheme key)
	if(!keySize)
	{
		// value should be entity scheme id and has appropriate size
		// if it not, think like there is no scheme
		ptr<EntityScheme> newScheme;
		if(value && value->GetSize() == EntitySchemeId::size)
			newScheme = manager->GetSchemeManager()->TryGet(EntitySchemeId::FromData(value->GetData()));

		// if scheme doesn't match
		if(scheme != newScheme)
		{
			// remember it
			scheme = newScheme;
			// fire callbacks
			for(size_t i = 0; i < callbacks.size(); ++i)
				callbacks[i]->FireScheme(scheme);
		}

		return;
	}

	// the rest is entity's tags, fields or data
	switch(*(const char*)keyData)
	{
	case 't':
		{
			if(keySize != EntityTagId::size + 1)
				break;
			EntityTagId tagId = EntityTagId::FromData((const char*)keyData + 1);
			for(size_t i = 0; i < callbacks.size(); ++i)
				callbacks[i]->FireTag(tagId, value);
		}
		break;
	case 'f':
		{
			int fieldIndex = TryParseFieldKey((const char*)keyData + 1, keySize - 1);
			if(fieldIndex < 0)
				break;
			for(size_t i = 0; i < callbacks.size(); ++i)
				callbacks[i]->FireField(fieldIndex, value);
		}
		break;
	case 'd':
		{
			ptr<File> key = MemoryFile::CreateViaCopy((const char*)keyData + 1, keySize - 1);
			for(size_t i = 0; i < callbacks.size(); ++i)
				callbacks[i]->FireData(key, value);
		}
		break;
	}
}

ptr<File> Entity::ReadTag(const EntityTagId& tagId) const
{
	if(!scheme)
		return nullptr;

	return manager->GetRepo()->GetValue(GetFullTagKey(tagId));
}

void Entity::WriteTag(ptr<Action> action, const EntityTagId& tagId, ptr<File> tagData)
{
	if(!scheme)
		return;

	action->AddChange(GetFullTagKey(tagId), tagData);
}

ptr<File> Entity::RawReadField(int fieldIndex) const
{
	if(!scheme)
		return nullptr;

	return manager->GetRepo()->GetValue(GetFullFieldKey(fieldIndex));
}

void Entity::RawWriteField(ptr<Action> action, int fieldIndex, ptr<File> value)
{
	if(!scheme)
		return;

	action->AddChange(GetFullFieldKey(fieldIndex), value);
}

void Entity::EnumerateFields(FieldEnumerator* enumerator)
{
	if(!scheme)
		return;

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

			enumerator->OnField(fieldIndex, value);
			return true;
		}
	};

	ptr<MemoryFile> prefix = NEW(MemoryFile(EntityId::size + 1));
	char* prefixData = (char*)prefix->GetData();
	memcpy(prefixData, id.data, EntityId::size);
	prefixData[EntityId::size] = 'f';

	manager->GetRepo()->EnumerateKeyValues(prefix, &Enumerator(enumerator));
}

ptr<Script::Any> Entity::ReadField(int fieldIndex) const
{
	if(!scheme)
		return nullptr;

	const EntityScheme::Fields& fields = scheme->GetFields();
	if(fieldIndex >= (int)fields.size())
		return nullptr;

	return fields[fieldIndex].type->TryConvertToScript(
		Script::Np::State::GetCurrent(),
		RawReadField(fieldIndex));
}

void Entity::WriteField(ptr<Action> action, int fieldIndex, ptr<Script::Any> value)
{
	if(!scheme)
		return;

	const EntityScheme::Fields& fields = scheme->GetFields();
	if(fieldIndex >= (int)fields.size())
		return;

	ptr<File> fileValue = fields[fieldIndex].type->TryConvertFromScript(value.FastCast<Script::Np::Any>());
	if(!fileValue)
		return;

	RawWriteField(action, fieldIndex, fileValue);
}

ptr<File> Entity::ReadData(const void* nameData, size_t nameSize) const
{
	if(!scheme)
		return nullptr;

	return manager->GetRepo()->GetValue(GetFullDataKey(nameData, nameSize));
}

void Entity::WriteData(ptr<Action> action, const void* nameData, size_t nameSize, ptr<File> value)
{
	if(!scheme)
		return;

	action->AddChange(GetFullDataKey(nameData, nameSize), value);
}

void Entity::EnumerateData(DataEnumerator* enumerator)
{
	if(!scheme)
		return;

	class Enumerator : public ClientRepo::KeyValueEnumerator
	{
	private:
		DataEnumerator* enumerator;

	public:
		Enumerator(DataEnumerator* enumerator)
		: enumerator(enumerator) {}

		bool OnKeyValue(ptr<File> key, ptr<File> value)
		{
			enumerator->OnData(key->SliceFrom(EntityId::size + 1), value);
			return true;
		}
	};

	ptr<MemoryFile> prefix = NEW(MemoryFile(EntityId::size + 1));
	char* prefixData = (char*)prefix->GetData();
	memcpy(prefixData, id.data, EntityId::size);
	prefixData[EntityId::size] = 'd';

	manager->GetRepo()->EnumerateKeyValues(prefix, &Enumerator(enumerator));
}

ptr<EntityCallback> Entity::AddCallback(ptr<Script::Any> callback)
{
	return NEW(EntityCallback(this, callback.FastCast<Script::Np::Any>()));
}

END_INANITY_OIL

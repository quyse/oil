#include "Entity.hpp"
#include "EntityScheme.hpp"
#include "EntityManager.hpp"
#include "EntitySchemeManager.hpp"
#include "EntityCallback.hpp"
#include "EntityInterface.hpp"
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
#ifdef _DEBUG
	for(size_t i = 0; i < callbacks.size(); ++i)
		if(callbacks[i] == callback)
			THROW("Entity callback already registered");
#endif
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

#ifdef _DEBUG
	THROW("Entity callback already freed");
#endif
}

void Entity::OnNewInterface(EntityInterface* interf)
{
	EntityInterfaceId interfaceId = interf->GetInterfaceId();
#ifdef _DEBUG
	if(interfaces.find(interfaceId) != interfaces.end())
		THROW("Entity interface already registered");
#endif
	interfaces[interfaceId] = interf;
}

void Entity::OnFreeInterface(EntityInterface* interf)
{
	Interfaces::iterator i = interfaces.find(interf->GetInterfaceId());
	if(i != interfaces.end())
		interfaces.erase(i);
#ifdef _DEBUG
	else
		THROW("Entity interface already freed");
#endif
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

ptr<File> Entity::GetFullFieldKey(const EntityFieldId& fieldId) const
{
	ptr<MemoryFile> key = NEW(MemoryFile(EntityId::size + 1 + EntityFieldId::size));
	char* keyData = (char*)key->GetData();
	memcpy(keyData, id.data, EntityId::size);
	keyData[EntityId::size] = 'f';
	memcpy(keyData + EntityId::size + 1, fieldId.data, EntityFieldId::size);

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

void Entity::OnChange(const void* keyData, size_t keySize, ptr<File> value)
{
	// if this is a main entity key (scheme key)
	if(!keySize)
	{
		// value should be entity scheme id and has appropriate size
		// if it not, think like there is no scheme
		ptr<EntityScheme> newScheme;
		if(value && value->GetSize() == EntitySchemeId::size)
			newScheme = manager->GetSchemeManager()->TryGetScheme(EntitySchemeId::FromData(value->GetData()));

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

	if(!scheme)
		return;

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
			if(keySize != EntityFieldId::size + 1)
				break;
			EntityFieldId fieldId = EntityFieldId::FromData((const char*)keyData + 1);
			for(size_t i = 0; i < callbacks.size(); ++i)
				callbacks[i]->FireField(fieldId, value);
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

ptr<File> Entity::RawReadField(const EntityFieldId& fieldId) const
{
	return manager->GetRepo()->GetValue(GetFullFieldKey(fieldId));
}

void Entity::RawWriteField(ptr<Action> action, const EntityFieldId& fieldId, ptr<File> value)
{
	action->AddChange(GetFullFieldKey(fieldId), value);
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
			// skip keys with wrong length
			if(key->GetSize() != EntityId::size + 1 + EntityFieldId::size)
				return true;

			enumerator->OnField(
				EntityFieldId::FromData((const char*)key->GetData() + EntityId::size + 1),
				value);
			return true;
		}
	};

	ptr<MemoryFile> prefix = NEW(MemoryFile(EntityId::size + 1));
	char* prefixData = (char*)prefix->GetData();
	memcpy(prefixData, id.data, EntityId::size);
	prefixData[EntityId::size] = 'f';

	manager->GetRepo()->EnumerateKeyValues(prefix, &Enumerator(enumerator));
}

ptr<Script::Any> Entity::ReadField(const EntityFieldId& fieldId) const
{
	if(!scheme)
		return nullptr;

	const EntityScheme::Fields& fields = scheme->GetFields();
	EntityScheme::Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		return nullptr;

	return i->second.type->TryConvertToScript(
		manager,
		Script::Np::State::GetCurrent(),
		RawReadField(fieldId));
}

void Entity::WriteField(ptr<Action> action, const EntityFieldId& fieldId, ptr<Script::Any> value)
{
	if(!scheme)
		return;

	const EntityScheme::Fields& fields = scheme->GetFields();
	EntityScheme::Fields::const_iterator i = fields.find(fieldId);
	if(i == fields.end())
		return;

	ptr<File> fileValue = i->second.type->TryConvertFromScript(value.FastCast<Script::Np::Any>());
	if(!fileValue)
		return;

	RawWriteField(action, fieldId, fileValue);
}

ptr<File> Entity::RawReadData(const void* nameData, size_t nameSize) const
{
	if(!scheme)
		return nullptr;

	return manager->GetRepo()->GetValue(GetFullDataKey(nameData, nameSize));
}

void Entity::RawWriteData(ptr<Action> action, const void* nameData, size_t nameSize, ptr<File> value)
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

ptr<File> Entity::ReadData(ptr<File> name) const
{
	return RawReadData(name ? name->GetData() : nullptr, name ? name->GetSize() : 0);
}

void Entity::WriteData(ptr<Action> action, ptr<File> name, ptr<File> value)
{
	RawWriteData(action, name ? name->GetData() : nullptr, name ? name->GetSize() : 0, value);
}

void Entity::Delete(ptr<Action> action)
{
	if(!scheme)
		return;

	class Enumerator : public ClientRepo::KeyEnumerator
	{
	private:
		ptr<Action> action;

	public:
		Enumerator(ptr<Action> action)
		: action(action) {}

		bool OnKey(ptr<File> key)
		{
			action->AddChange(key, nullptr);
			return true;
		}
	};

	manager->GetRepo()->EnumerateKeys(id.ToFile(), &Enumerator(action));
}

ptr<EntityCallback> Entity::AddCallback(ptr<Script::Any> callback)
{
	return NEW(EntityCallback(this, callback.FastCast<Script::Np::Any>()));
}

ptr<EntityInterface> Entity::GetInterface(const EntityInterfaceId& interfaceId)
{
	BEGIN_TRY();

	// try to find an interface
	Interfaces::const_iterator i = interfaces.find(interfaceId);
	if(i != interfaces.end())
		return i->second;

	// so there is no interface object
	// create new
	ptr<EntityInterface> interf = NEW(EntityInterface(this, interfaceId));

	return interf;

	END_TRY("Can't get entity interface");
}

void Entity::SetInterfaceResult(const EntityInterfaceId& interfaceId, ptr<Script::Any> result)
{
	// try to find an interface
	Interfaces::const_iterator i = interfaces.find(interfaceId);
	if(i != interfaces.end())
		i->second->SetResult(result);
}

END_INANITY_OIL

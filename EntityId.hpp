#ifndef ___INANITY_OIL_ENTITY_ID_HPP___
#define ___INANITY_OIL_ENTITY_ID_HPP___

#include "oil.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_OIL

struct EntityId
{
	static const size_t size = 16;
	unsigned char data[size];

	String ToString() const;
	ptr<File> ToFile() const;

	// for script
	static ptr<File> StaticToFile(const EntityId& id);

	static EntityId FromString(const char* string);
	static EntityId FromData(const void* data);
	static EntityId FromFile(ptr<File> file);
	static EntityId New();

	friend bool operator<(const EntityId& a, const EntityId& b);
};

typedef EntityId EntitySchemeId;
typedef EntityId EntityInterfaceId;
typedef EntityId EntityTagId;

END_INANITY_OIL

#endif

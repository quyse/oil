#ifndef ___INANITY_OIL_ID_HPP___
#define ___INANITY_OIL_ID_HPP___

#include "oil.hpp"
#include "../inanity/String.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_OIL

template <int n, typename Derived>
struct Id
{
	static const size_t size = n;
	unsigned char data[size];

	String ToString() const;
	ptr<File> ToFile() const;

	// for script
	static ptr<File> StaticToFile(const Derived& id);
	static Derived FromStringData(const String& string);

	static Derived FromString(const String& string);
	static Derived FromData(const void* data);
	static Derived FromFile(ptr<File> file);
};

template <int n, typename Derived>
bool operator<(const Id<n, Derived>& a, const Id<n, Derived>& b);

struct EntityId;
extern template struct Id<16, EntityId>;
struct EntityId : public Id<16, EntityId>
{
	static EntityId New();
};
extern template bool operator<(const Id<16, EntityId>& a, const Id<16, EntityId>& b);

struct EntitySchemeId;
extern template struct Id<4, EntitySchemeId>;
struct EntitySchemeId : public Id<4, EntitySchemeId>
{
};
extern template bool operator<(const Id<4, EntitySchemeId>& a, const Id<4, EntitySchemeId>& b);

struct EntityInterfaceId;
extern template struct Id<4, EntityInterfaceId>;
struct EntityInterfaceId : public Id<4, EntityInterfaceId>
{
};
extern template bool operator<(const Id<4, EntityInterfaceId>& a, const Id<4, EntityInterfaceId>& b);

struct EntityTagId;
extern template struct Id<4, EntityTagId>;
struct EntityTagId : public Id<4, EntityTagId>
{
};
extern template bool operator<(const Id<4, EntityTagId>& a, const Id<4, EntityTagId>& b);

struct EntityFieldId;
extern template struct Id<4, EntityFieldId>;
struct EntityFieldId : public Id<4, EntityFieldId>
{
};
extern template bool operator<(const Id<4, EntityFieldId>& a, const Id<4, EntityFieldId>& b);

struct EntityTagIds
{
	static EntityTagId name;
	static EntityTagId description;
	static EntityTagId parent;
};

END_INANITY_OIL

#endif

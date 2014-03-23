#include "Id.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"
#include <cstring>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

BEGIN_INANITY_OIL

template <int n, typename Derived>
String Id<n, Derived>::ToString() const
{
	String s(n * 2, ' ');
	static const char hex[] = "0123456789abcdef";
	for(int i = 0; i < n; ++i)
	{
		s[i * 2] = hex[data[i] >> 4];
		s[i * 2 + 1] = hex[data[i] & 0x0f];
	}
	return s;
}

template <int n, typename Derived>
ptr<File> Id<n, Derived>::ToFile() const
{
	return MemoryFile::CreateViaCopy(data, size);
}

template <int n, typename Derived>
ptr<File> Id<n, Derived>::StaticToFile(const Derived& id)
{
	return id.ToFile();
}

template <int n, typename Derived>
Derived Id<n, Derived>::FromStringData(const String& string)
{
	if(string.length() != n)
		THROW(String("Wrong string data length for ") + typeid(Derived).name());
	return FromData(string.c_str());
}

inline unsigned char fromHex(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return 10 + c - 'a';
	if(c >= 'A' && c <= 'F')
		return 10 + c - 'A';
	THROW("Wrong hex symbol");
}

template <int n, typename Derived>
Derived Id<n, Derived>::FromString(const String& string)
{
	if(string.length() != n * 2)
		THROW(String("Wrong length of id string for ") + typeid(Derived).name());

	Derived id;

	for(int i = 0; i < n; ++i)
		id.data[i] = (fromHex(string[i * 2]) << 4) | fromHex(string[i * 2 + 1]);

	return id;
}

template <int n, typename Derived>
Derived Id<n, Derived>::FromData(const void* data)
{
	Derived id;
	memcpy(id.data, data, size);

	return id;
}

template <int n, typename Derived>
Derived Id<n, Derived>::FromFile(ptr<File> file)
{
	if(file->GetSize() != size)
		THROW(String("Wrong size of id file for ") + typeid(Derived).name());

	return FromData(file->GetData());
}

EntityId EntityId::New()
{
	static auto generator = boost::uuids::random_generator();

	boost::uuids::uuid uuid = generator();

	EntityId id;
	std::copy(uuid.begin(), uuid.end(), id.data);

	return id;
}

template <int n, typename Derived>
bool operator<(const Id<n, Derived>& a, const Id<n, Derived>& b)
{
	return memcmp(a.data, b.data, Id<n, Derived>::size) < 0;
}

template struct Id<16, EntityId>;
template bool operator<(const Id<16, EntityId>& a, const Id<16, EntityId>& b);
template struct Id<4, EntitySchemeId>;
template bool operator<(const Id<4, EntitySchemeId>& a, const Id<4, EntitySchemeId>& b);
template struct Id<4, EntityInterfaceId>;
template bool operator<(const Id<4, EntityInterfaceId>& a, const Id<4, EntityInterfaceId>& b);
template struct Id<4, EntityTagId>;
template bool operator<(const Id<4, EntityTagId>& a, const Id<4, EntityTagId>& b);
template struct Id<4, EntityFieldId>;
template bool operator<(const Id<4, EntityFieldId>& a, const Id<4, EntityFieldId>& b);

EntityTagId EntityTagIds::name = EntityTagId::FromData("name");
EntityTagId EntityTagIds::description = EntityTagId::FromData("desc");
EntityTagId EntityTagIds::parent = EntityTagId::FromData("pare");

END_INANITY_OIL

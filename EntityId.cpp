#include "EntityId.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"
#include <cstring>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

BEGIN_INANITY_OIL

bool operator<(const EntityId& a, const EntityId& b)
{
	return memcmp(a.data, b.data, EntityId::size) < 0;
}

String EntityId::ToString() const
{
	try
	{
		boost::uuids::uuid uuid;
		std::copy(data, data + size, uuid.begin());
		return boost::lexical_cast<String>(uuid);
	}
	catch(const boost::bad_lexical_cast&)
	{
		THROW("Can't convert entity id to string");
	}
}

ptr<File> EntityId::ToFile() const
{
	return MemoryFile::CreateViaCopy(data, size);
}

EntityId EntityId::FromString(const char* data)
{
	try
	{
		boost::uuids::uuid uuid = boost::lexical_cast<boost::uuids::uuid>(data);

		EntityId id;
		std::copy(uuid.begin(), uuid.end(), id.data);

		return id;
	}
	catch(const boost::bad_lexical_cast&)
	{
		THROW("Can't create entity id from string");
	}
}

EntityId EntityId::FromData(const void* data)
{
	EntityId id;
	memcpy(id.data, data, size);

	return id;
}

EntityId EntityId::FromFile(ptr<File> file)
{
	if(file->GetSize() != size)
		THROW("Wrong size of entity ID file");

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

END_INANITY_OIL

#ifndef ___INANITY_OIL_ENTITY_SCHEME_HPP___
#define ___INANITY_OIL_ENTITY_SCHEME_HPP___

#include "Id.hpp"
#include <map>
#include <set>
#include <vector>

BEGIN_INANITY_OIL

class Entity;
class EntityFieldType;
class EntityInterface;

/// Scheme of fields in entity.
class EntityScheme : public Object
{
public:
	struct Field
	{
		ptr<EntityFieldType> type;
		String name;
	};
	typedef std::map<EntityFieldId, Field> Fields;
	typedef std::set<ptr<EntityInterface> > Interfaces;

private:
	EntitySchemeId id;
	String name;

	/// Fields of scheme.
	Fields fields;
	/// Fields of scheme by index.
	std::vector<EntityFieldId> fieldIds;

	/// Interfaces supported by scheme.
	Interfaces interfaces;

public:
	EntityScheme(const EntitySchemeId& id, const String& name);

	EntitySchemeId GetId() const;
	String GetName() const;

	const Fields& GetFields() const;
	const Interfaces& GetInterfaces() const;

	//** for scripts
	int GetFieldsCount() const;
	EntityFieldId GetFieldId(int index) const;
	ptr<EntityFieldType> GetFieldType(const EntityFieldId& fieldId) const;
	String GetFieldName(const EntityFieldId& fieldId) const;
	void AddField(const EntityFieldId& fieldId, ptr<EntityFieldType> type, const String& name);
	void AddInterface(ptr<EntityInterface> entityInterface);
};

END_INANITY_OIL

#endif

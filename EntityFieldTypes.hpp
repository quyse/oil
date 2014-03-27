#ifndef ___INANITY_OIL_ENTITY_FIELD_TYPES_HPP___
#define ___INANITY_OIL_ENTITY_FIELD_TYPES_HPP___

#include "EntityFieldType.hpp"
#include "Id.hpp"
#include <set>

BEGIN_INANITY_OIL

class Entity;
class EntityInterface;

class BlobEntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class BoolEntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class FloatEntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class IntegerEntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class StringEntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class Vec3EntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class Vec4EntityFieldType : public EntityFieldType
{
public:
	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class Color3EntityFieldType : public Vec3EntityFieldType
{
public:
	String GetName() const;
};

class Color4EntityFieldType : public Vec4EntityFieldType
{
public:
	String GetName() const;
};

class ReferenceEntityFieldType : public EntityFieldType
{
private:
	typedef std::set<ptr<EntityInterface> > Interfaces;
	/// Interfaces entity should have.
	Interfaces interfaces;

public:
	Interfaces& GetInterfaces();

	/// Check if entity could be referenced.
	bool CheckEntity(ptr<Entity> entity) const;

	String GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);

	//*** for scripts
	void AddInterface(ptr<EntityInterface> interf);
};

END_INANITY_OIL

#endif

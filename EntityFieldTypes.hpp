#ifndef ___INANITY_OIL_ENTITY_FIELD_TYPES_HPP___
#define ___INANITY_OIL_ENTITY_FIELD_TYPES_HPP___

#include "EntityFieldType.hpp"

BEGIN_INANITY_OIL

class FloatEntityFieldType : public EntityFieldType
{
public:
	const char* GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class IntegerEntityFieldType : public EntityFieldType
{
public:
	const char* GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class StringEntityFieldType : public EntityFieldType
{
public:
	const char* GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class Vec3EntityFieldType : public EntityFieldType
{
public:
	const char* GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class Vec4EntityFieldType : public EntityFieldType
{
public:
	const char* GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

class Color3EntityFieldType : public Vec3EntityFieldType
{
public:
	const char* GetName() const;
};

class Color4EntityFieldType : public Vec4EntityFieldType
{
public:
	const char* GetName() const;
};

class ReferenceEntityFieldType : public EntityFieldType
{
public:
	const char* GetName() const;
	ptr<Script::Any> TryConvertToScript(EntityManager* entityManager, ptr<Script::Np::State> scriptState, ptr<File> value);
	ptr<File> TryConvertFromScript(ptr<Script::Np::Any> value);
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_ENTITY_CALLBACK_HPP___
#define ___INANITY_OIL_ENTITY_CALLBACK_HPP___

#include "Id.hpp"
#include "../inanity/script/np/np.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_SCRIPT

class Any;

END_INANITY_SCRIPT

BEGIN_INANITY_NP

class Any;

END_INANITY_NP

BEGIN_INANITY_OIL

class Entity;
class EntityScheme;

class EntityCallback : public Object
{
private:
	ptr<Entity> entity;
	/// Callback for monitoring changes.
	ptr<Script::Np::Any> callback;

public:
	EntityCallback(ptr<Entity> entity, ptr<Script::Np::Any> callback);
	~EntityCallback();

	void FireScheme(ptr<EntityScheme> scheme);
	void FireTag(const EntityTagId& tagId, ptr<File> value);
	void FireField(const EntityFieldId& fieldId, ptr<File> value);
	void FireData(ptr<File> key, ptr<File> value);

	/// Fire callback for all fields.
	void EnumerateFields();
	/// Fire callback for all data.
	void EnumerateData();
};

END_INANITY_OIL

#endif

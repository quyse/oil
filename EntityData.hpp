#ifndef ___INANITY_OIL_ENTITY_DATA_HPP___
#define ___INANITY_OIL_ENTITY_DATA_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class EntityScheme;

/// Abstract entity data object.
/** Depends on a scheme. */
class EntityData : public Object
{
private:
	ptr<EntityScheme> scheme;

public:
	EntityData(ptr<EntityScheme> scheme);

	ptr<EntityScheme> GetScheme() const;

	virtual void OnChange(
		const void* keyData, size_t keySize,
		const void* valueData, size_t valueSize) = 0;
};

END_INANITY_OIL

#endif

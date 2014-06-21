#ifndef ___INANITY_OIL_MATERIAL_INSTANCE_HPP___
#define ___INANITY_OIL_MATERIAL_INSTANCE_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class Material;

/// Abstract class of instance of material.
/** Controls presentation and animation of material. */
class MaterialInstance : public Object
{
protected:
	ptr<Material> material;

public:
	MaterialInstance(ptr<Material> material);
	~MaterialInstance();

	ptr<Material> GetMaterial() const;
};

END_INANITY_OIL

#endif

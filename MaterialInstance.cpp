#include "MaterialInstance.hpp"
#include "Material.hpp"

BEGIN_INANITY_OIL

MaterialInstance::MaterialInstance(ptr<Material> material)
: material(material) {}

MaterialInstance::~MaterialInstance() {}

ptr<Material> MaterialInstance::GetMaterial() const
{
	return material;
}

END_INANITY_OIL

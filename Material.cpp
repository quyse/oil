#include "Material.hpp"
#include "../inanity/graphics/Texture.hpp"

BEGIN_INANITY_OIL

Material::Material() {}

Material::~Material() {}

ptr<Graphics::Texture> Material::GetDiffuseTexture() const
{
	return diffuseTexture;
}

ptr<ColorController> Material::GetDiffuseController() const
{
	return diffuseController;
}

ptr<Graphics::Texture> Material::GetNormalTexture() const
{
	return normalTexture;
}

END_INANITY_OIL

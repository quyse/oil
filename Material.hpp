#ifndef ___INANITY_OIL_MATERIAL_HPP___
#define ___INANITY_OIL_MATERIAL_HPP___

#include "Controller.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_GRAPHICS

class Texture;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

/// Material class.
class Material : public Object
{
private:
	ptr<Graphics::Texture> diffuseTexture;
	ptr<ColorController> diffuseController;
	ptr<Graphics::Texture> normalTexture;

public:
	Material();
	~Material();

	ptr<Graphics::Texture> GetDiffuseTexture() const;
	ptr<ColorController> GetDiffuseController() const;
	ptr<Graphics::Texture> GetNormalTexture() const;
};

END_INANITY_OIL

#endif

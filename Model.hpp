#ifndef ___INANITY_OIL_MODEL_HPP___
#define ___INANITY_OIL_MODEL_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_GRAPHICS

class RawMesh;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

class Material;

class Model : public Object
{
private:
	ptr<Graphics::RawMesh> mesh;
	ptr<Material> material;

public:
	Model(ptr<Graphics::RawMesh> mesh, ptr<Material> material);
	~Model();

	ptr<Graphics::RawMesh> GetMesh() const;
	ptr<Material> GetMaterial() const;
};

END_INANITY_OIL

#endif

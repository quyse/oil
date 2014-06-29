#include "Model.hpp"
#include "Material.hpp"
#include "../inanity/graphics/RawMesh.hpp"

BEGIN_INANITY_OIL

Model::Model(ptr<Graphics::RawMesh> mesh, ptr<Material> material)
: mesh(mesh), material(material) {}

Model::~Model() {}

ptr<Graphics::RawMesh> Model::GetMesh() const
{
	return mesh;
}

ptr<Material> Model::GetMaterial() const
{
	return material;
}

END_INANITY_OIL

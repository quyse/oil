#include "Model.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

BEGIN_INANITY_OIL

Model::Model(ptr<Mesh> mesh, ptr<Material> material)
: mesh(mesh), material(material) {}

Model::~Model() {}

ptr<Mesh> Model::GetMesh() const
{
	return mesh;
}

ptr<Material> Model::GetMaterial() const
{
	return material;
}

END_INANITY_OIL

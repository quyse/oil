#include "MeshInstance.hpp"
#include "Mesh.hpp"

BEGIN_INANITY_OIL

MeshInstance::MeshInstance(ptr<Mesh> mesh)
: mesh(mesh) {}

MeshInstance::~MeshInstance() {}

ptr<Mesh> MeshInstance::GetMesh() const
{
	return mesh;
}

END_INANITY_OIL

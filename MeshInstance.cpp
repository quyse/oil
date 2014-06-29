#include "MeshInstance.hpp"
#include "../inanity/graphics/RawMesh.hpp"

BEGIN_INANITY_OIL

MeshInstance::MeshInstance(ptr<Graphics::RawMesh> mesh)
: mesh(mesh) {}

MeshInstance::~MeshInstance() {}

ptr<Graphics::RawMesh> MeshInstance::GetMesh() const
{
	return mesh;
}

END_INANITY_OIL

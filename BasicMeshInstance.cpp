#include "BasicMeshInstance.hpp"
#include "Painter.hpp"
#include "../inanity/graphics/RawMesh.hpp"

BEGIN_INANITY_OIL

BasicMeshInstance::BasicMeshInstance(ptr<Graphics::RawMesh> mesh, ptr<TransformController> transformController)
: MeshInstance(mesh), transformController(transformController) {}

BasicMeshInstance::~BasicMeshInstance() {}

ptr<TransformController> BasicMeshInstance::GetTransformController() const
{
	return transformController;
}

void BasicMeshInstance::Register(Painter* painter)
{
}

END_INANITY_OIL

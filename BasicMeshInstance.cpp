#include "BasicMeshInstance.hpp"
#include "Mesh.hpp"
#include "Painter.hpp"

BEGIN_INANITY_OIL

BasicMeshInstance::BasicMeshInstance(ptr<Mesh> mesh, ptr<TransformController> transformController)
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

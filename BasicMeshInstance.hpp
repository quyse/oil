#ifndef ___INANITY_OIL_BASIC_MESH_INSTANCE_HPP___
#define ___INANITY_OIL_BASIC_MESH_INSTANCE_HPP___

#include "MeshInstance.hpp"
#include "Controller.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_OIL

/// Class representing just mesh with transform.
class BasicMeshInstance : public MeshInstance
{
private:
	ptr<TransformController> transformController;

public:
	BasicMeshInstance(ptr<Graphics::RawMesh> mesh, ptr<TransformController> transformController);
	~BasicMeshInstance();

	ptr<TransformController> GetTransformController() const;

	//*** MeshInstance methods.
	void Register(Painter* painter);
};

END_INANITY_OIL

#endif

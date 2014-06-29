#ifndef ___INANITY_OIL_MESH_INSTANCE_HPP___
#define ___INANITY_OIL_MESH_INSTANCE_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_GRAPHICS

class RawMesh;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

class Painter;

/// Abstract class representing instance of mesh in scene.
class MeshInstance : public Object
{
protected:
	ptr<Graphics::RawMesh> mesh;

public:
	MeshInstance(ptr<Graphics::RawMesh> mesh);
	~MeshInstance();

	ptr<Graphics::RawMesh> GetMesh() const;

	virtual void Register(Painter* painter) = 0;
};

END_INANITY_OIL

#endif

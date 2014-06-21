#ifndef ___INANITY_OIL_MESH_INSTANCE_HPP___
#define ___INANITY_OIL_MESH_INSTANCE_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class Mesh;
class Painter;

/// Abstract class representing instance of mesh in scene.
class MeshInstance : public Object
{
protected:
	ptr<Mesh> mesh;

public:
	MeshInstance(ptr<Mesh> mesh);
	~MeshInstance();

	ptr<Mesh> GetMesh() const;

	virtual void Register(Painter* painter) = 0;
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_MODEL_HPP___
#define ___INANITY_OIL_MODEL_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class Mesh;
class Material;

class Model : public Object
{
private:
	ptr<Mesh> mesh;
	ptr<Material> material;

public:
	Model(ptr<Mesh> mesh, ptr<Material> material);
	~Model();

	ptr<Mesh> GetMesh() const;
	ptr<Material> GetMaterial() const;
};

END_INANITY_OIL

#endif

#ifndef ___INANITY_OIL_MODEL_INSTANCE_HPP___
#define ___INANITY_OIL_MODEL_INSTANCE_HPP___

#include "oil.hpp"

BEGIN_INANITY_OIL

class Model;
class MeshInstance;
class MaterialInstance;
class Painter;

/// Instance of model in the scene.
class ModelInstance : public Object
{
protected:
	ptr<Model> model;
	ptr<MeshInstance> meshInstance;
	ptr<MaterialInstance> materialInstance;

public:
	ModelInstance(ptr<Model> model, ptr<MeshInstance> meshInstance, ptr<MaterialInstance> materialInstance);
	~ModelInstance();

	ptr<Model> GetModel() const;
	ptr<MeshInstance> GetGeometryInstance() const;
	ptr<MaterialInstance> GetMaterialInstance() const;

	void Register(Painter* painter);
};

END_INANITY_OIL

#endif

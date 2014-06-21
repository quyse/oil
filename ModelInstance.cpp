#include "ModelInstance.hpp"
#include "Model.hpp"
#include "MeshInstance.hpp"
#include "MaterialInstance.hpp"
#include "Painter.hpp"

BEGIN_INANITY_OIL

ModelInstance::ModelInstance(ptr<Model> model, ptr<MeshInstance> meshInstance, ptr<MaterialInstance> materialInstance)
: model(model), meshInstance(meshInstance), materialInstance(materialInstance) {}

ModelInstance::~ModelInstance() {}

ptr<Model> ModelInstance::GetModel() const
{
	return model;
}

ptr<MeshInstance> ModelInstance::GetGeometryInstance() const
{
	return meshInstance;
}

ptr<MaterialInstance> ModelInstance::GetMaterialInstance() const
{
	return materialInstance;
}

void ModelInstance::Register(Painter* painter)
{
}

END_INANITY_OIL

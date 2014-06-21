#include "ImportedScene.hpp"
#include "Model.hpp"
#include "Material.hpp"

BEGIN_INANITY_OIL

ImportedScene::ImportedScene() {}

ImportedScene::~ImportedScene() {}

ImportedScene::ModelsByName& ImportedScene::GetModelsByName()
{
	return modelsByName;
}

ImportedScene::MaterialsByName& ImportedScene::GetMaterialsByName()
{
	return materialsByName;
}

ptr<Model> ImportedScene::TryGetModel(const String& name) const
{
	ModelsByName::const_iterator i = modelsByName.find(name);
	if(i == modelsByName.end())
		return nullptr;
	return i->second;
}

ptr<Material> ImportedScene::TryGetMaterial(const String& name) const
{
	MaterialsByName::const_iterator i = materialsByName.find(name);
	if(i == materialsByName.end())
		return nullptr;
	return i->second;
}

END_INANITY_OIL

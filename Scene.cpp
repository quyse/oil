#include "Scene.hpp"
#include "Model.hpp"

BEGIN_INANITY_OIL

Scene::Scene() {}

Scene::~Scene() {}

Scene::Models& Scene::GetModels()
{
	return models;
}

END_INANITY_OIL

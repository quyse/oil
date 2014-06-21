#ifndef ___INANITY_OIL_IMPORTED_SCENE_HPP___
#define ___INANITY_OIL_IMPORTED_SCENE_HPP___

#include "Scene.hpp"
#include "../inanity/String.hpp"
#include <unordered_map>

BEGIN_INANITY_OIL

class Model;
class Material;

/// Extends Scene with additional methods
/// for gathering and enumerating scene objects.
class ImportedScene : public Scene
{
public:
	typedef std::unordered_map<String, ptr<Model> > ModelsByName;
	typedef std::unordered_map<String, ptr<Material> > MaterialsByName;

private:
	ModelsByName modelsByName;
	MaterialsByName materialsByName;

public:
	ImportedScene();
	~ImportedScene();

	ModelsByName& GetModelsByName();
	MaterialsByName& GetMaterialsByName();

	ptr<Model> TryGetModel(const String& name) const;
	ptr<Material> TryGetMaterial(const String& materials) const;
};

END_INANITY_OIL

#endif

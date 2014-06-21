#ifndef ___INANITY_OIL_ASSIMP_SCENE_IMPORTER_HPP___
#define ___INANITY_OIL_ASSIMP_SCENE_IMPORTER_HPP___

#include "SceneImporter.hpp"

BEGIN_INANITY_OIL

class AssimpSceneImporter : public SceneImporter
{
public:
	AssimpSceneImporter();
	~AssimpSceneImporter();

	//*** SceneImporter's methods.
	ptr<ImportedScene> Import(ptr<File> file);
};

END_INANITY_OIL

#endif

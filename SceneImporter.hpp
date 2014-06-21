#ifndef ___INANITY_OIL_SCENE_IMPORTER_HPP___
#define ___INANITY_OIL_SCENE_IMPORTER_HPP___

#include "oil.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_OIL

class ImportedScene;

/// Abstract scene importer class.
class SceneImporter : public Object
{
public:
	virtual ptr<ImportedScene> Import(ptr<File> file) = 0;
};

END_INANITY_OIL

#endif

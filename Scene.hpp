#ifndef ___INANITY_OIL_SCENE_HPP___
#define ___INANITY_OIL_SCENE_HPP___

#include "oil.hpp"
#include <vector>

BEGIN_INANITY_OIL

class Model;

/// Class representing scene with 3D objects.
class Scene : public Object
{
public:
	typedef std::vector<ptr<Model> > Models;

protected:
	Models models;

public:
	Scene();
	~Scene();

	Models& GetModels();
};

END_INANITY_OIL

#endif

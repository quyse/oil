#ifndef ___INANITY_OIL_VIEW_SCRIPT_OBJECT_HPP___
#define ___INANITY_OIL_VIEW_SCRIPT_OBJECT_HPP___

#include "oil.hpp"
#include <map>

BEGIN_INANITY_OIL

class ViewPluginInstance;
class ViewRenderer;

/// Script object for 'view' plugin instance.
class ViewScriptObject : public Object
{
private:
	ViewPluginInstance* pluginInstance;

	//*** Workaround for wrapper problem.
	// Plugin object's methods accept objects created
	// by main plugin instance, wrapping them into special
	// Mozilla class. It prevents from converting them to
	// Inanity objects. To workaround, script gets the object
	// by another channel - via static method with id, so
	// object is created "by main instance", and no wrapping
	// happens.
	int id;
	typedef std::map<int, ViewScriptObject*> Ids;
	static Ids ids;
	static int nextId;

public:
	ViewScriptObject(ViewPluginInstance* pluginInstance);
	~ViewScriptObject();

	int GetId() const;
	static ptr<ViewScriptObject> GetById(int id);

	void Invalidate();
	void SetViewRenderer(ptr<ViewRenderer> viewRenderer);
};

END_INANITY_OIL

#endif

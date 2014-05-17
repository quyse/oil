#include "ViewScriptObject.hpp"
#include "ViewPluginInstance.hpp"
#include "ViewRenderer.hpp"

BEGIN_INANITY_OIL

ViewScriptObject::Ids ViewScriptObject::ids;
int ViewScriptObject::nextId = 1;

ViewScriptObject::ViewScriptObject(ViewPluginInstance* pluginInstance)
: pluginInstance(pluginInstance)
{
	id = nextId++;
	ids[id] = this;
}

ViewScriptObject::~ViewScriptObject()
{
	ids.erase(id);
}

int ViewScriptObject::GetId() const
{
	return id;
}

ptr<ViewScriptObject> ViewScriptObject::GetById(int id)
{
	Ids::const_iterator i = ids.find(id);
	return i != ids.end() ? i->second : nullptr;
}

void ViewScriptObject::Invalidate()
{
	pluginInstance->Invalidate();
}

void ViewScriptObject::SetViewRenderer(ptr<ViewRenderer> viewRenderer)
{
	pluginInstance->SetViewRenderer(viewRenderer);
}

END_INANITY_OIL

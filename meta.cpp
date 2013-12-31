#include "../inanity/script/np/impl.ipp"
#include "../inanity/inanity-base-meta.ipp"
#include "../inanity/inanity-graphics-meta.ipp"
#include "../inanity/inanity-physics-meta.ipp"
#include "../inanity/inanity-math-script.ipp"

#include "ScriptObject.hpp"
META_CLASS(Inanity::Oil::ScriptObject, Inanity.Oil.ScriptObject);
	META_METHOD(GetRootNamespace);
	META_METHOD(GetNativeFileSystem);
META_CLASS_END();

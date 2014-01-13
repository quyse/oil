#include "../inanity/script/np/impl.ipp"
#include "../inanity/inanity-base-meta.ipp"
#include "../inanity/inanity-graphics-meta.ipp"
#include "../inanity/inanity-physics-meta.ipp"
#include "../inanity/inanity-math-script.ipp"

#include "ScriptObject.hpp"
META_CLASS(Inanity::Oil::ScriptObject, Inanity.Oil.ScriptObject);
	META_METHOD(GetRootNamespace);
	META_METHOD(GetNativeFileSystem);
	META_METHOD(CreateLocalClientRepo);
	META_METHOD(CreateTempClientRepo);
	META_METHOD(CreateMemoryClientRepo);
	META_METHOD(CreateUrlRemoteRepo);
	META_METHOD(CreateLocalRemoteRepo);
	META_METHOD(CreateTempRemoteRepo);
	META_METHOD(CreateMemoryRemoteRepo);
	META_METHOD(CreateScriptRepo);
META_CLASS_END();

#include "RemoteRepo.hpp"
META_CLASS(Inanity::Oil::RemoteRepo, Inanity.Oil.RemoteRepo);
META_CLASS_END();

#include "ClientRepo.hpp"
META_CLASS(Inanity::Oil::ClientRepo, Inanity.Oil.ClientRepo);
META_CLASS_END();

#include "ScriptRepo.hpp"
META_CLASS(Inanity::Oil::ScriptRepo, Inanity.Oil.ScriptRepo);
	META_METHOD(Init);
META_CLASS_END();

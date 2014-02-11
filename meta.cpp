#include "../inanity/script/np/impl.ipp"
#include "../inanity/inanity-base-meta.ipp"
#include "../inanity/inanity-graphics-meta.ipp"
#include "../inanity/inanity-physics-meta.ipp"
#include "../inanity/inanity-math-script.ipp"


//*** Convertors for valuetypes.

#include "EntityId.hpp"

BEGIN_INANITY_SCRIPT

template <>
struct ConverterFromScript<Oil::EntityId>
{
	static Oil::EntityId Convert(ptr<Any> value)
	{
		return Oil::EntityId::FromString(value->AsString().c_str());
	}
};

template <>
struct ConverterToScript<Oil::EntityId>
{
	static ptr<Any> Convert(ptr<State> state, const Oil::EntityId& value)
	{
		return state->NewString(value.ToString());
	}
};

END_INANITY_SCRIPT


//*** Classes.

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
	META_METHOD(GetEntityManager);
	META_METHOD(Init);
	META_METHOD(Sync);
	META_METHOD(Watch);
	META_METHOD(ProcessEvents);
	META_METHOD(MakeAction);
	META_METHOD(Undo);
	META_METHOD(Redo);
	META_METHOD(SetUndoRedoChangedCallback);
	META_METHOD(GetPushLag);
	META_METHOD(GetPushedKeysCount);
	META_METHOD(GetPullLag);
	META_METHOD(GetPulledKeysCount);
META_CLASS_END();

#include "Action.hpp"
META_CLASS(Inanity::Oil::Action, Inanity.Oil.Action);
	META_CONSTRUCTOR(const String&);
	META_METHOD(GetDescription);
	META_METHOD(AddChange);
META_CLASS_END();

#include "EntityId.hpp"
META_CLASS(Inanity::Oil::EntityId, Inanity.Oil.EntityId);
	META_STATIC_METHOD(StaticToFile);
	META_STATIC_METHOD(FromFile);
	META_STATIC_METHOD(New);
META_CLASS_END();

#include "Entity.hpp"
META_CLASS(Inanity::Oil::Entity, Inanity.Oil.Entity);
	META_METHOD(GetManager);
	META_METHOD(GetId);
	META_METHOD(GetScheme);
	META_METHOD(ReadTag);
	META_METHOD(WriteTag);
	META_METHOD(RawReadField);
	META_METHOD(RawWriteField);
	META_METHOD(ReadField);
	META_METHOD(WriteField);
	META_METHOD(AddCallback);
META_CLASS_END();

#include "EntityManager.hpp"
META_CLASS(Inanity::Oil::EntityManager, Inanity.Oil.EntityManager);
	META_METHOD(GetRepo);
	META_METHOD(GetSchemeManager);
	META_METHOD(CreateEntity);
	META_METHOD(GetEntity);
META_CLASS_END();

#include "EntityScheme.hpp"
META_CLASS(Inanity::Oil::EntityScheme, Inanity.Oil.EntityScheme);
	META_CONSTRUCTOR(const Oil::EntitySchemeId&, const String&);
	META_METHOD(GetId);
	META_METHOD(GetName);
	META_METHOD(GetFieldsCount);
	META_METHOD(GetFieldType);
	META_METHOD(GetFieldName);
	META_METHOD(AddField);
META_CLASS_END();

#include "EntitySchemeManager.hpp"
META_CLASS(Inanity::Oil::EntitySchemeManager, Inanity.Oil.EntitySchemeManager);
	META_METHOD(TryGet);
	META_METHOD(Get);
	META_METHOD(Register);
META_CLASS_END();

#include "EntityCallback.hpp"
META_CLASS(Inanity::Oil::EntityCallback, Inanity.Oil.EntityCallback);
	META_METHOD(EnumerateFields);
	META_METHOD(EnumerateData);
META_CLASS_END();

#include "../inanity/script/np/impl.ipp"
#include "../inanity/inanity-base-meta.ipp"
#include "../inanity/inanity-graphics-meta.ipp"
#include "../inanity/inanity-physics-meta.ipp"
#include "../inanity/inanity-math-script.ipp"


//*** Convertors for valuetypes.

#include "Id.hpp"

BEGIN_INANITY_SCRIPT

#define ID_CONVERTER(T) \
	template <> \
	struct ConverterFromScript<T> \
	{ \
		static T Convert(ptr<Any> value) \
		{ \
			return T::FromString(value->AsString()); \
		} \
	}; \
	template <> \
	struct ConverterToScript<T> \
	{ \
		static ptr<Any> Convert(ptr<State> state, const T& value) \
		{ \
			return state->NewString(value.ToString()); \
		} \
	}

ID_CONVERTER(Inanity::Oil::EntityId);
ID_CONVERTER(Inanity::Oil::EntitySchemeId);
ID_CONVERTER(Inanity::Oil::EntityInterfaceId);
ID_CONVERTER(Inanity::Oil::EntityTagId);
ID_CONVERTER(Inanity::Oil::EntityFieldId);

#undef ID_CONVERTER

END_INANITY_SCRIPT


//*** Classes.

#include "ScriptObject.hpp"
META_CLASS(Inanity::Oil::ScriptObject, Inanity.Oil.ScriptObject);
	META_METHOD(GetRootNamespace);
	META_METHOD(GetNativeFileSystem);
	META_METHOD(GetProfileFileSystem);
	META_METHOD(SetProfilePath);
	META_METHOD(Init);
	META_METHOD(CreateLocalClientRepo);
	META_METHOD(CreateTempClientRepo);
	META_METHOD(CreateMemoryClientRepo);
	META_METHOD(CreateUrlRemoteRepo);
	META_METHOD(CreateLocalRemoteRepo);
	META_METHOD(CreateTempRemoteRepo);
	META_METHOD(CreateMemoryRemoteRepo);
	META_METHOD(CreateScriptRepo);
META_CLASS_END();

#include "ViewScriptObject.hpp"
META_CLASS(Inanity::Oil::ViewScriptObject, Inanity.Oil.ViewScriptObject);
	META_METHOD(Invalidate);
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

//******* ids

META_CLASS(Inanity::Oil::EntityId, Inanity.Oil.EntityId);
	META_STATIC_METHOD(StaticToFile);
	META_STATIC_METHOD(FromFile);
	META_STATIC_METHOD(New);
META_CLASS_END();

META_CLASS(Inanity::Oil::EntitySchemeId, Inanity.Oil.EntitySchemeId);
	META_STATIC_METHOD(StaticToFile);
	META_STATIC_METHOD(FromStringData);
	META_STATIC_METHOD(FromFile);
META_CLASS_END();

META_CLASS(Inanity::Oil::EntityInterfaceId, Inanity.Oil.EntityInterfaceId);
	META_STATIC_METHOD(StaticToFile);
	META_STATIC_METHOD(FromStringData);
	META_STATIC_METHOD(FromFile);
META_CLASS_END();

META_CLASS(Inanity::Oil::EntityTagId, Inanity.Oil.EntityTagId);
	META_STATIC_METHOD(StaticToFile);
	META_STATIC_METHOD(FromStringData);
	META_STATIC_METHOD(FromFile);
META_CLASS_END();

META_CLASS(Inanity::Oil::EntityFieldId, Inanity.Oil.EntityFieldId);
	META_STATIC_METHOD(StaticToFile);
	META_STATIC_METHOD(FromStringData);
	META_STATIC_METHOD(FromFile);
META_CLASS_END();

//*******

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
	META_METHOD(ReadData);
	META_METHOD(WriteData);
	META_METHOD(Delete);
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
	META_METHOD(GetFieldId);
	META_METHOD(GetFieldType);
	META_METHOD(GetFieldName);
	META_METHOD(AddField);
	META_METHOD(AddInterface);
META_CLASS_END();

#include "EntityInterface.hpp"
META_CLASS(Inanity::Oil::EntityInterface, Inanity.Oil.EntityInterface);
	META_METHOD(GetEntity);
	META_METHOD(GetInterfaceId);
META_CLASS_END();

#include "EntitySchemeManager.hpp"
META_CLASS(Inanity::Oil::EntitySchemeManager, Inanity.Oil.EntitySchemeManager);
	META_METHOD(TryGetScheme);
	META_METHOD(GetScheme);
	META_METHOD(RegisterScheme);
	META_METHOD(GetStandardFieldType);
META_CLASS_END();

#include "EntityCallback.hpp"
META_CLASS(Inanity::Oil::EntityCallback, Inanity.Oil.EntityCallback);
	META_METHOD(EnumerateFields);
	META_METHOD(EnumerateData);
META_CLASS_END();

#include "EntityInterfaceCallback.hpp"
META_CLASS(Inanity::Oil::EntityInterfaceCallback, Inanity.Oil.EntityInterfaceCallback);
META_CLASS_END();

#include "EntityFieldType.hpp"
META_CLASS(Inanity::Oil::EntityFieldType, Inanity.Oil.EntityFieldType);
	META_METHOD(GetName);
META_CLASS_END();

#include "EntityFieldTypes.hpp"

META_CLASS(Inanity::Oil::ReferenceEntityFieldType, Inanity.Oil.ReferenceEntityFieldType);
	META_CLASS_PARENT(Inanity::Oil::EntityFieldType);
	META_CONSTRUCTOR();
	META_METHOD(CheckEntity);
	META_METHOD(AddInterface);
META_CLASS_END();

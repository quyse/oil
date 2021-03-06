#include "../inanity/script/np/impl.ipp"
#include "../inanity/inanity-base-meta.ipp"
#include "../inanity/inanity-data-meta.ipp"
#include "../inanity/inanity-graphics-meta.ipp"
#include "../inanity/inanity-physics-meta.ipp"
#include "../inanity/inanity-math-script.ipp"


//*** Convertors for valuetypes.

#include "Id.hpp"

BEGIN_INANITY_SCRIPT

#define ID_CONVERTER(T) \
	template <> \
	struct ConverterFromScript<META_PROVIDER, T> \
	{ \
		static T Convert(ptr<Any> value) \
		{ \
			return T::FromString(value->AsString()); \
		} \
	}; \
	template <> \
	struct ConverterToScript<META_PROVIDER, T> \
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

#include "Core.hpp"
META_CLASS(Inanity::Oil::Core, Inanity.Oil.Core);
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
	META_METHOD(GetEngine);
META_CLASS_END();

#include "Material.hpp"

META_CLASS(Inanity::Oil::Material, Inanity.Oil.Material);
	META_METHOD(GetDiffuseTexture);
	META_METHOD(GetDiffuseController);
	META_METHOD(GetNormalTexture);
META_CLASS_END();

#include "Model.hpp"

META_CLASS(Inanity::Oil::Model, Inanity.Oil.Model);
META_CLASS_END();

#include "Scene.hpp"

META_CLASS(Inanity::Oil::Scene, Inanity.Oil.Scene);
META_CLASS_END();

#include "ImportedScene.hpp"

META_CLASS(Inanity::Oil::ImportedScene, Inanity.Oil.ImportedScene);
	META_CLASS_PARENT(Inanity::Oil::Scene);
	META_METHOD(TryGetModel);
	META_METHOD(TryGetMaterial);
META_CLASS_END();

#include "Engine.hpp"
META_CLASS(Inanity::Oil::Engine, Inanity.Oil.Engine);
	META_METHOD(LoadRawTexture);
META_CLASS_END();

#include "ViewRenderer.hpp"
META_CLASS(Inanity::Oil::ViewRenderer, Inanity.Oil.ViewRenderer);
META_CLASS_END();

#include "ViewScriptObject.hpp"
META_CLASS(Inanity::Oil::ViewScriptObject, Inanity.Oil.ViewScriptObject);
	META_METHOD(GetId);
	META_STATIC_METHOD(GetById);
	META_METHOD(Invalidate);
	META_METHOD(SetViewRenderer);
META_CLASS_END();

#include "TextureViewRenderer.hpp"
META_CLASS(Inanity::Oil::TextureViewRenderer, Inanity.Oil.TextureViewRenderer);
	META_CLASS_PARENT(Inanity::Oil::ViewRenderer);
	META_CONSTRUCTOR(ptr<Inanity::Oil::Engine>);
	META_METHOD(SetTexture);
	META_METHOD(SetScale);
	META_METHOD(SetOffset);
	META_METHOD(SetFilter);
	META_METHOD(SetTile);
	META_METHOD(SetMipMode);
	META_METHOD(SetMipLod);
	META_METHOD(SetMipBias);
	META_METHOD(SetColorTransform);
	META_METHOD(SetColorOffset);
META_CLASS_END();

#include "SceneViewRenderer.hpp"
META_CLASS(Inanity::Oil::SceneViewRenderer, Inanity.Oil.SceneViewRenderer);
	META_CLASS_PARENT(Inanity::Oil::ViewRenderer);
	META_CONSTRUCTOR(ptr<Inanity::Oil::Engine>);
META_CLASS_END();

#include "RemoteRepo.hpp"
META_CLASS(Inanity::Oil::RemoteRepo, Inanity.Oil.RemoteRepo);
META_CLASS_END();

#include "Repo.hpp"
META_CLASS(Inanity::Oil::Repo, Inanity.Oil.Repo);
	META_METHOD(Vacuum);
	META_METHOD(IntegrityCheck);
META_CLASS_END();

#include "ClientRepo.hpp"
META_CLASS(Inanity::Oil::ClientRepo, Inanity.Oil.ClientRepo);
	META_CLASS_PARENT(Inanity::Oil::Repo);
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
	META_METHOD(GetClientRepo);
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

//******* controllers

#include "Controller.hpp"
#include "ConstantController.hpp"

#define META_OF_CONTROLLER(name) \
	META_CLASS(Inanity::Oil::name##Controller, Inanity.Oil.name##Controller); \
		META_STATIC_METHOD(CreateConstant); \
	META_CLASS_END()

META_OF_CONTROLLER(Vec3);
META_OF_CONTROLLER(Vec4);
META_OF_CONTROLLER(Mat4x4);

#undef META_OF_CONTROLLER

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
	META_METHOD(WriteDataStraight);
	META_METHOD(Delete);
	META_METHOD(AddCallback);
	META_METHOD(GetInterface);
	META_METHOD(SetInterfaceResult);
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
	META_METHOD(GetInterfacesCount);
	META_METHOD(GetInterfaceId);
	META_METHOD(HasInterface);
	META_METHOD(AddInterface);
META_CLASS_END();

#include "EntityInterface.hpp"
META_CLASS(Inanity::Oil::EntityInterface, Inanity.Oil.EntityInterface);
	META_METHOD(GetEntity);
	META_METHOD(GetInterfaceId);
	META_METHOD(AddCallback);
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
	META_METHOD(EnumerateScheme);
	META_METHOD(EnumerateTag);
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
META_CLASS_END();

#include "FileEntityScheme.hpp"

META_CLASS(Inanity::Oil::FileEntitySchemeInputStream, Inanity.Oil.FileEntitySchemeInputStream);
	META_CLASS_PARENT(Inanity::InputStream);
	META_CONSTRUCTOR(ptr<Inanity::Oil::Entity>);
	META_METHOD(GetSize);
META_CLASS_END();

META_CLASS(Inanity::Oil::FileEntitySchemeOutputStream, Inanity.Oil.FileEntitySchemeOutputStream);
	META_CLASS_PARENT(Inanity::OutputStream);
	META_CONSTRUCTOR(ptr<Inanity::Oil::Action>, ptr<Inanity::Oil::Entity>, size_t);
	META_METHOD(End);
META_CLASS_END();

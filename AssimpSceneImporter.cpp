#include "AssimpSceneImporter.hpp"
#include "Model.hpp"
#include "ImportedScene.hpp"
#include "../inanity/File.hpp"
#include "../inanity/deps/assimp/repo/include/assimp/Importer.hpp"
#include "../inanity/deps/assimp/repo/include/assimp/scene.h"
#include "../inanity/deps/assimp/repo/include/assimp/mesh.h"

BEGIN_INANITY_OIL

AssimpSceneImporter::AssimpSceneImporter() {}

AssimpSceneImporter::~AssimpSceneImporter() {}

ptr<ImportedScene> AssimpSceneImporter::Import(ptr<File> file)
{
	BEGIN_TRY();

	Assimp::Importer importer;

	const aiScene* aScene = importer.ReadFileFromMemory(file->GetData(), file->GetSize(),
		// calculate tangents and binormals
		aiProcess_CalcTangentSpace |
		// this flag is required for index buffer
		aiProcess_JoinIdenticalVertices |
		// only 3-vertex faces
		aiProcess_Triangulate |
		// optimize order of vertices
		aiProcess_ImproveCacheLocality |
		// sort by primitive type
		aiProcess_SortByPType |
		// get right Y uv coord
		aiProcess_FlipUVs |
		// CW order
		aiProcess_FlipWindingOrder
	);
	if(!aScene)
		THROW(String("Assimp failed: ") + importer.GetErrorString());

	ptr<ImportedScene> scene = NEW(ImportedScene());
	ImportedScene::Models& models = scene->GetModels();

	// assimp calls models "meshes"
	// so assimp's "mesh" is really a mesh + material, i.e. model

	models.resize(aScene->mNumMeshes);
	for(size_t i = 0; i < models.size(); ++i)
	{
		aiMesh* aMesh = aScene->mMeshes[i];

		size_t verticesCount = aMesh->mNumVertices;
		aiVector3D* positions = aMesh->mVertices;
		aiVector3D* normals = aMesh->mNormals;
		aiVector3D* tangents = aMesh->mTangents;
		aiVector3D* bitangents = aMesh->mBitangents;
		aiVector3D* textureCoords = aMesh->mTextureCoords[0];
	}

	return scene;

	END_TRY("Can't import scene with assimp");
}

END_INANITY_OIL

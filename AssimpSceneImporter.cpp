#include "AssimpSceneImporter.hpp"
#include "Model.hpp"
#include "ImportedScene.hpp"
#include "Material.hpp"
#include "../inanity/graphics/RawMesh.hpp"
#include "../inanity/graphics/VertexLayout.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/Exception.hpp"
#include "../inanity/deps/assimp/repo/include/assimp/Importer.hpp"
#include "../inanity/deps/assimp/repo/include/assimp/scene.h"
#include "../inanity/deps/assimp/repo/include/assimp/mesh.h"
#include "../inanity/deps/assimp/repo/include/assimp/postprocess.h"

using namespace Inanity::Graphics;

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
		const aiMesh* aMesh = aScene->mMeshes[i];

		// convert vertices

		size_t verticesCount = aMesh->mNumVertices;
		const aiVector3D* positions = aMesh->mVertices;
		const aiVector3D* tangents = aMesh->mTangents;
		const aiVector3D* bitangents = aMesh->mBitangents;
		const aiVector3D* normals = aMesh->mNormals;
		const aiVector3D* textureCoords = aMesh->mTextureCoords[0];

		struct Vertex
		{
			vec3 position;
			vec3 tangent;
			vec3 binormal;
			vec3 normal;
			vec2 texcoord;
		};

		auto c3 = [](const aiVector3D& v) { return vec3(v.x, v.y, v.z); };
		auto c2 = [](const aiVector3D& v) { return vec2(v.x, v.y); };

		ptr<MemoryFile> verticesFile = NEW(MemoryFile(verticesCount * sizeof(Vertex)));
		Vertex* vertices = (Vertex*)verticesFile->GetData();
		for(size_t i = 0; i < verticesCount; ++i)
		{
			Vertex& vertex = vertices[i];
			vertex.position = c3(positions[i]);
			vertex.tangent = c3(tangents[i]);
			vertex.binormal = c3(bitangents[i]);
			vertex.normal = c3(normals[i]);
			vertex.texcoord = c2(textureCoords[i]);
		}

		// convert indices

		size_t facesCount = aMesh->mNumFaces;
		const aiFace* faces = aMesh->mFaces;

		size_t indicesCount = facesCount * 3;
		ptr<MemoryFile> indicesFile;
		// if we have to use big indices
		if(indicesCount > 0x10000)
		{
			indicesFile = NEW(MemoryFile(facesCount * 3 * sizeof(unsigned int)));
			unsigned int* indices = (unsigned int*)indicesFile->GetData();
			for(size_t i = 0; i < facesCount; ++i)
			{
				const aiFace& face = faces[i];
				for(size_t j = 0; j < 3; ++j)
					indices[i * 3 + j] = face.mIndices[j];
			}
		}
		// else we can use small indices
		else
		{
			indicesFile = NEW(MemoryFile(facesCount * 3 * sizeof(unsigned short)));
			unsigned short* indices = (unsigned short*)indicesFile->GetData();
			for(size_t i = 0; i < facesCount; ++i)
			{
				const aiFace& face = faces[i];
				for(size_t j = 0; j < 3; ++j)
					indices[i * 3 + j] = (unsigned short)face.mIndices[j];
			}
		}

		models.push_back(NEW(Model(NEW(RawMesh(verticesFile, nullptr, indicesFile)), nullptr)));
	}

	return scene;

	END_TRY("Can't import scene with assimp");
}

END_INANITY_OIL

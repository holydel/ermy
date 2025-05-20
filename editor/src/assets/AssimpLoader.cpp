#pragma once
#include "assets/AssimpLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post-processing flags
#include <ermy_log.h>
#include <ermy_utils.h>
#include "assets/geometry_asset.h"

Assimp::Importer* gAssetImporter = nullptr;

using namespace editor::asset::loader;
using namespace ermy;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& AssimpLoader::SupportedExtensions()
{
	return gSupportedExtensions;
}

// Function to extract all extensions from the input string
static std::vector<std::string> extractExtensions(const std::string& input) {
	std::vector<std::string> extensions;

	// Step 1: Split the input string by ';' to get individual patterns
	std::vector<std::string> patterns = ermy_utils::string::split(input, ';');

	for (const std::string& pattern : patterns) {
		// Step 2: Remove the '*.' prefix to get the pure extension
		if (pattern.size() > 2 && pattern[0] == '*' && pattern[1] == '.') {
			extensions.push_back(pattern.substr(2)); // Extract substring after '*.'
		}
	}

	return extensions;
}


AssimpLoader::AssimpLoader()
{
	gAssetImporter = new Assimp::Importer();

	aiString supportedGeometryExtensions;
	gAssetImporter->GetExtensionList(supportedGeometryExtensions);

	ERMY_LOG(u8"Supported geometry extenions list: %s\n", supportedGeometryExtensions.C_Str());
	gSupportedExtensions = extractExtensions(supportedGeometryExtensions.C_Str());

	gAssetImporter->SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, UINT16_MAX);
}

AssimpLoader::~AssimpLoader()
{
	delete gAssetImporter;
}

AssetData* AssimpLoader::Load(const std::filesystem::path& path)
{
	const aiScene* scene = gAssetImporter->ReadFile(path.string().c_str(),
		aiProcess_Triangulate | aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality | aiProcess_CalcTangentSpace |
		aiProcess_SortByPType | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_GenBoundingBoxes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERMY_ERROR(u8"Cant load geometry by path: %s", path.string().c_str());
		return nullptr;
	}
	GeometryAsset* result = new GeometryAsset();

	u32 totalVertices = 0;
	u32 totalIndices = 0;

	for (u32 m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];

		totalVertices += mesh->mNumVertices;
		totalIndices += mesh->mNumFaces * 3;
	}

	result->numVertices = totalVertices;
	result->numIndices = totalIndices;

	result->allVertices.resize(totalVertices);
	result->allIndices.resize(totalIndices);
	memset(result->allVertices.data(), 0, sizeof(rendering::StaticVertexDedicated) * totalVertices);

	u32 currentIndexOffset = 0;
	u32 currentVertexOffset = 0;

	// Process the first mesh (assuming the model has at least one mesh)
	for (u32 m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];
		bool hasTangents = mesh->HasTangentsAndBitangents();
		u32 numColorChannels = mesh->GetNumColorChannels();
		u32 numUVChannels = mesh->GetNumUVChannels();

		auto hasBones = mesh->HasBones();
		auto hasFaces = mesh->HasFaces();
		auto hasNormals = mesh->HasNormals();
		auto hasPositions = mesh->HasPositions();

		auto bbox = mesh->mAABB;

		ermy::rendering::SubMesh submesh;
		submesh.indexCount = mesh->mNumFaces * 3;
		submesh.vertexCount = mesh->mNumVertices;
		submesh.indexOffset = currentIndexOffset;
		submesh.vertexOffset = currentVertexOffset;
		for (int f = 0; f < mesh->mNumFaces; ++f)
		{
			int i = f * 3;
			result->allIndices[currentIndexOffset + i + 0] = mesh->mFaces[f].mIndices[0];
			result->allIndices[currentIndexOffset + i + 1] = mesh->mFaces[f].mIndices[1];
			result->allIndices[currentIndexOffset + i + 2] = mesh->mFaces[f].mIndices[2];
		}

		for (int i = 0; i < mesh->mNumVertices; ++i)
		{			
			auto& v = result->allVertices[currentVertexOffset + i];

			if (hasPositions)
			{
				v.x = mesh->mVertices[i].x;
				v.y = mesh->mVertices[i].y;
				v.z = mesh->mVertices[i].z;
			}
			
			if (hasNormals)
			{
				v.nx = mesh->mNormals[i].x;
				v.ny = mesh->mNormals[i].y;
				v.nz = mesh->mNormals[i].z;
			}
			
			if (hasTangents)
			{
				v.tx = mesh->mTangents[i].x;
				v.ty = mesh->mTangents[i].y;
				v.tz = mesh->mTangents[i].z;

				v.bx = mesh->mBitangents[i].x;
				v.by = mesh->mBitangents[i].y;
				v.bz = mesh->mBitangents[i].z;
			}

			if (numColorChannels > 0)
			{
				v.r = mesh->mColors[0]->r;
				v.g = mesh->mColors[0]->g;
				v.b = mesh->mColors[0]->b;
				v.a = mesh->mColors[0]->a;
			}

			if (numUVChannels > 0)
			{
				v.u0 = mesh->mTextureCoords[0]->x;
				v.v0 = mesh->mTextureCoords[0]->y;
				
				if (numUVChannels > 1)
				{
					v.u1 = mesh->mTextureCoords[1]->x;
					v.v1 = mesh->mTextureCoords[1]->y;
				}
			}
		}

		result->subMeshes.push_back(submesh);

		currentIndexOffset += submesh.indexCount;
		currentVertexOffset += submesh.vertexCount;
		//mesh->HasTextureCoords();
		//mesh->HasTextureCoordsName();
		//mesh->HasTextureCoordsName();
	}

	result->RegenerateLivePreview();

	return result;
}
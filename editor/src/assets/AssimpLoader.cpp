#pragma once
#include "assets/AssimpLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post-processing flags
#include <ermy_log.h>
#include <ermy_utils.h>
Assimp::Importer* gAssetImporter = nullptr;

using namespace editor::asset::loader;

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
	
	ERMY_LOG("Supported geometry extenions list: %s\n", supportedGeometryExtensions.C_Str());
    gSupportedExtensions = extractExtensions(supportedGeometryExtensions.C_Str());
}

AssimpLoader::~AssimpLoader()
{
	delete gAssetImporter;
}

AssetData* AssimpLoader::Load(const std::filesystem::path& path)
{
	//const aiScene* scene = gAssetImporter->ReadFile(filepath,
//	aiProcess_Triangulate | aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality | aiProcess_CalcTangentSpace |
//	aiProcess_SortByPType | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_GenBoundingBoxes);

//if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
//	ERMY_ERROR("Cant load geometry by path: %s", filepath);
//	return;
//}

//// Process the first mesh (assuming the model has at least one mesh)
//for (u32 m=0;m<scene->mNumMeshes;++m)
//{
//	aiMesh* mesh = scene->mMeshes[m];
//	bool hasTangents = mesh->HasTangentsAndBitangents();
//	u32 numColorChannels = mesh->GetNumColorChannels();
//	u32 numUVChannels = mesh->GetNumUVChannels();
//	
//	auto bones = mesh->HasBones();
//	auto faces = mesh->HasFaces();
//	auto normals = mesh->HasNormals();
//	auto positions = mesh->HasPositions();
//	auto bbox = mesh->mAABB;
//	//mesh->HasTextureCoords();
//	//mesh->HasTextureCoordsName();
//	//mesh->HasTextureCoordsName();
//}

	return nullptr;
}
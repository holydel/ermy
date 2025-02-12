#include <assets/geometry_asset.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post-processing flags
#include <ermy_log.h>

Assimp::Importer* gAssetImporter = nullptr;
using namespace ermy;

GeometryAsset::GeometryAsset(const char* filepath)
{
	const aiScene* scene = gAssetImporter->ReadFile(filepath,
		aiProcess_Triangulate | aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality | aiProcess_CalcTangentSpace |
		aiProcess_SortByPType | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_GenBoundingBoxes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERMY_ERROR("Cant load geometry by path: %s", filepath);
		return;
	}

	// Process the first mesh (assuming the model has at least one mesh)
	for (u32 m=0;m<scene->mNumMeshes;++m)
	{
		aiMesh* mesh = scene->mMeshes[m];
		bool hasTangents = mesh->HasTangentsAndBitangents();
		u32 numColorChannels = mesh->GetNumColorChannels();
		u32 numUVChannels = mesh->GetNumUVChannels();
		
		auto bones = mesh->HasBones();
		auto faces = mesh->HasFaces();
		auto normals = mesh->HasNormals();
		auto positions = mesh->HasPositions();
		auto bbox = mesh->mAABB;
		//mesh->HasTextureCoords();
		//mesh->HasTextureCoordsName();
		//mesh->HasTextureCoordsName();
	}
}

GeometryAsset::~GeometryAsset()
{

}

std::vector<std::string>  GeometryAsset::Initialize()
{
	gAssetImporter = new Assimp::Importer();

	aiString supportedGeometryExtensions;
	gAssetImporter->GetExtensionList(supportedGeometryExtensions);
	
	ERMY_LOG("Supported geometry extenions list: %s\n", supportedGeometryExtensions.C_Str());

	std::vector<std::string> result;
	return result;
}

void GeometryAsset::Shutdown()
{
	delete gAssetImporter;
}
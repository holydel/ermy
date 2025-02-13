#include <assets/geometry_asset.h>
#include <ermy_log.h>

//Assimp::Importer* gAssetImporter = nullptr;
using namespace ermy;

GeometryAsset::GeometryAsset()
{

}

GeometryAsset::~GeometryAsset()
{

}

//std::vector<std::string>  GeometryAsset::Initialize()
//{
//	gAssetImporter = new Assimp::Importer();
//
//	aiString supportedGeometryExtensions;
//	gAssetImporter->GetExtensionList(supportedGeometryExtensions);
//	
//	ERMY_LOG("Supported geometry extenions list: %s\n", supportedGeometryExtensions.C_Str());
//
//	std::vector<std::string> result;
//	return result;
//}
//
//void GeometryAsset::Shutdown()
//{
//	delete gAssetImporter;
//}
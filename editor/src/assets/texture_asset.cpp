#include <assets/texture_asset.h>
#include <ermy_log.h>
#include <compressonator.h>
#include <imgui.h>
#include <ermy_utils.h>

TextureAsset::TextureAsset()
{
   
}

TextureAsset::~TextureAsset()
{

}

void TextureAsset::DrawPreview()
{
	ImGui::Text("Width: %d Height: %d",width,height);
	ImGui::Text("Datasize: %s",ermy_utils::string::humanReadableFileSize(dataSize).c_str());
}

void TextureAsset::RegeneratePreview()
{
	ermy::rendering::TextureDesc desc;
	desc.width = width;
	desc.height = height;
	desc.depth = depth;
	desc.isCubemap = isCubemap;
	desc.numLayers = numLayers;
	desc.numMips = numMips;
	desc.pixelsData = data;
	desc.isSparse = false;

	previewTexture = ermy::rendering::CreateDedicatedTexture(desc);
	assetPreviewTex = ermy::rendering::GetTextureDescriptor(previewTexture);
}

//std::vector<std::string>  TextureAsset::Initialize()
//{
//    std::string supported_formats;
//    getattribute("input_format_list", supported_formats);
//    ERMY_LOG("Supported image formats: %s\n", supported_formats.c_str());
//    std::cout << "Supported image formats: " << supported_formats << std::endl;
//
//    std::vector<std::string> result;
//    return result;
//}
//
//void TextureAsset::Shutdown()
//{
//
//}
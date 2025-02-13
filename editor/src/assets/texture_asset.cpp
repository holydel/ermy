#include <assets/texture_asset.h>
#include <ermy_log.h>
#include <compressonator.h>
#include <imgui.h>

TextureAsset::TextureAsset()
{
   
}

TextureAsset::~TextureAsset()
{

}

void TextureAsset::DrawProps()
{
	ImGui::Text("Some texture");
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
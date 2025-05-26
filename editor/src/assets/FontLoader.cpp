#pragma once
#include "assets/FontLoader.h"

#include "assets/font_asset.h"

using namespace editor::asset::loader;
using namespace ermy;

static std::vector<std::string> gSupportedExtensions;

const std::vector<std::string>& FontLoader::SupportedExtensions()
{
	gSupportedExtensions.push_back("ttf");
	gSupportedExtensions.push_back("ttc");
	gSupportedExtensions.push_back("otf");
	gSupportedExtensions.push_back("otc");
	gSupportedExtensions.push_back("pfa");
	gSupportedExtensions.push_back("pfb");
	gSupportedExtensions.push_back("cff");
	gSupportedExtensions.push_back("woff");
	gSupportedExtensions.push_back("cid");
	gSupportedExtensions.push_back("type42");
	
	//bitmap
	gSupportedExtensions.push_back("fnt");
	gSupportedExtensions.push_back("bdf");
	gSupportedExtensions.push_back("pcf");
	gSupportedExtensions.push_back("pfr");
	return gSupportedExtensions;
}


FontLoader::FontLoader()
{

}

FontLoader::~FontLoader()
{

}

AssetData* FontLoader::Load(const std::filesystem::path& path)
{
	FontAsset* result = new FontAsset();
	result->font = ermy::Font::CreateFromFile(path.u8string().c_str());

	ImGuiIO& io = ImGui::GetIO();

	//result->imFont = io.Fonts->AddFontFromFileTTF((const char*)path.u8string().c_str(), 24);

	result->RegenerateLivePreview();

	return result;
}
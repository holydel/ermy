#include "font_interface.h"
#include <freetype/freetype.h>
#include <ermy_application.h>

using namespace ermy;


Font* Font::CreateFromFile(const char8_t* u8filename, Font::AtlasType atlasType)
{
	FontImpl* result = new FontImpl(atlasType);
	result->LoadTTFFont(u8filename);
	
	return result;
}

FontImpl::FontImpl(Font::AtlasType atlasTypeIn)
	:atlasType(atlasTypeIn)
{
	ermy::rendering::Format atlasTexelFormat = ermy::rendering::Format::RGBA8_UNORM;
	switch (atlasType)
	{
	case Font::AtlasType::Grayscale:
	case Font::AtlasType::SDF:
		atlasTexelFormat = ermy::rendering::Format::R8_UNORM;
		break;

	case Font::AtlasType::MSDFT:
		atlasTexelFormat = ermy::rendering::Format::RGBA8_UNORM;
		break;
	}

	const auto& fontCfg = Application::GetApplication()->staticConfig.render.fontConfig;

	rendering::TextureDesc tdesc = rendering::TextureDesc::Create2D(fontCfg.initialTextureSize, fontCfg.initialTextureSize, atlasTexelFormat);
	atlasID = rendering::CreateDedicatedTexture(tdesc);
}

FontImpl::~FontImpl()
{

}

rendering::TextureID FontImpl::GetAtlas()
{
	return atlasID;
}

void FontImpl::FillTexCoords(const char8_t* u8string, std::vector<Font::SpriteData>& out)
{
	out.clear();
}

void FontImpl::LoadTTFFont(const char8_t* u8filename)
{

}
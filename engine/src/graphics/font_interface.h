#pragma once
#include <ermy_font.h>

class FontImpl : public ermy::Font
{
    ermy::rendering::TextureID atlasID;
    Font::AtlasType atlasType = Font::AtlasType::Grayscale;
public:
    FontImpl(Font::AtlasType atlasType);
    ~FontImpl();

    ermy::rendering::TextureID GetAtlas() override;
    void FillTexCoords(const char8_t* u8string, std::vector<SpriteData>& out) override;

    void LoadTTFFont(const char8_t* u8filename);
};
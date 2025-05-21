#pragma once
#include <ermy_font.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <msdfgen.h>
#include <ext/import-font.h>

#include <unordered_map>

class FontImpl : public ermy::Font
{
    ermy::rendering::TextureID atlasID;
    Font::AtlasType atlasType = Font::AtlasType::Grayscale;
	FT_Face face_ft = nullptr;
    msdfgen::FontHandle* font_msdf = nullptr;

	std::unordered_map<uint32_t, GlyphData> glyphs;

	void RenderGlyph(uint32_t codepoint);
public:
    FontImpl(Font::AtlasType atlasType);
    ~FontImpl();

    ermy::rendering::TextureID GetAtlas() override;
    void FillTexCoords(const char8_t* u8string, std::vector<GlyphData>& out) override;

    void LoadTTFFont(const char8_t* u8filename);

    virtual ermy::u32 GetNumberOfGlyphs() override;

    virtual ermy::Font::FullAtlasInfo GenerateFullAtlas(int glyphSize) override;
};
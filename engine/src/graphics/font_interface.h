#pragma once
#include <ermy_font.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <msdfgen.h>
#include <ext/import-font.h>

#include <unordered_map>

class FontImpl;

class AtlasImpl : public ermy::Font::Atlas
{
    ermy::rendering::TextureID atlasID;
    ermy::Font::AtlasType atlasType = ermy::Font::AtlasType::Grayscale;
    int glyphSize = 24;
    FontImpl* font = nullptr;
public:
    AtlasImpl(FontImpl* hostFont, ermy::Font::AtlasType type, int glyphSizeIn);
    ~AtlasImpl();
    virtual void FillTexCoords(const char8_t* u8string, std::vector<ermy::Font::GlyphAtlasData>& out) override;
    virtual ermy::Font::FullAtlasInfo GenerateFullAtlas() override;
    virtual ermy::rendering::TextureID GetAtlas() override
    {
        return atlasID;
    }
};

class VertexCacheImpl : public ermy::Font::VertexCache
{
    ermy::rendering::BufferID vertexBufferID;
    ermy::rendering::BufferID indexBufferID;
    FontImpl* font = nullptr;
public:
    VertexCacheImpl(FontImpl* hostFont);
    ~VertexCacheImpl();
    virtual void FillMeshData(const char8_t* u8string, ermy::Font::MeshType meshType, std::vector<ermy::Font::GlyphMeshData>& out) override;
    virtual ermy::Font::FullVertexCacheInfo GenerateFullVertexCache() override;
    virtual ermy::rendering::BufferID GetIndexBuffer() override
    {
        return indexBufferID;
    }
    virtual ermy::rendering::BufferID GetVertexBuffer() override
    {
        return vertexBufferID;
    }
};

class FontImpl : public ermy::Font
{
    friend class AtlasImpl;
    friend class VertexCacheImpl;

	FT_Face face_ft = nullptr;
    msdfgen::FontHandle* font_msdf = nullptr;

	std::unordered_map<uint32_t, GlyphAtlasData> glyphs;
public:
    FontImpl();
    ~FontImpl();

    void LoadTTFFont(const char8_t* u8filename);

    virtual ermy::u32 GetNumberOfGlyphs() override;

    virtual std::vector<ermy::u8> GeneratePreviewRGBA(int width, int height) override;

    virtual Atlas* CreateAtlas(AtlasType type, int glyphSize) override
    {
        return new AtlasImpl(this, type, glyphSize);
    }

    virtual VertexCache* CreateVertexCache() override
    {
        return new VertexCacheImpl(this);
    }
};
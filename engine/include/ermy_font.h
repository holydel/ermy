#pragma once
#include "ermy_rendering.h"
#include <glm/glm.hpp>
#include <unordered_map>
namespace ermy
{
    class Font
    {
    public:
        struct FontVertex
        {
            //TODO: move to half float
            float x, y, z;
            float nx, ny, nz;
            float u, v;
        };

        enum class AtlasType : u8
        {
            Grayscale,
            SDF,
            MSDFT
        };

        enum class MeshType : u8
        {
            OutlineMesh2D,
            OutlineMesh3D,
        };

        struct GlyphAtlasData
        {
            u16 u0;
            u16 v0;
            u16 u1;
            u16 v1;

            u16 advance = 0;
            u16 bearingX = 0;
            u16 bearingY = 0;

            glm::vec4 TexCoords()
            {
                constexpr float v = 1.0f / 65535.0f;

                return glm::vec4(static_cast<float>(u0) * v,
                    static_cast<float>(v0) * v,
                    static_cast<float>(u1) * v,
                    static_cast<float>(v1) * v);
            }

            float Advance() { return static_cast<float>(advance) / 64.0f; }
            float BearingX() { return static_cast<float>(bearingX) / 64.0f; }
            float BearingY() { return static_cast<float>(bearingY) / 64.0f; }

        };

        struct GlyphMeshData
        {
            u32 indexOffset = 0;
            u32 indexCount = 0;
			u32 vertexOffset = 0;

			// in EM units (1/64.0f of a unit)
            glm::i16vec2 offset;

            glm::vec2 GetOffset() const
            {
                return glm::vec2(static_cast<float>(offset.x) / 64.0f, static_cast<float>(offset.y) / 64.0f);
			}
		};

        struct FullAtlasInfo
        {
            std::vector<std::byte> pixelsData;

            u16 width;
            u16 height;
            ermy::rendering::Format format;

            std::unordered_map<char32_t, GlyphAtlasData> allCachedGlyphsTex;
        };

        struct ASyncAtlasResult
        {
            volatile float progress = 0.0f;
            volatile bool isDone = false;

            //access result ONLY if isDone is true
            FullAtlasInfo result = {};
        };

        struct FullVertexCacheInfo
        {
            std::vector<u16> meshIndices;
            std::unordered_map<char32_t, GlyphMeshData> allCachedGlyphsMesh;
        };

        class Atlas
        {
        public:
            virtual void FillTexCoords(const char8_t* u8string, std::vector<GlyphAtlasData>& out) = 0;
            virtual FullAtlasInfo GenerateFullAtlas() = 0;
            virtual bool GenerateFullAtlasASync(ASyncAtlasResult* resultOut) = 0;
            virtual rendering::TextureID GetAtlas() = 0;
        };

        class VertexCache
        {
        public:
            virtual void FillMeshData(const char8_t* u8string, MeshType meshType, std::vector<GlyphMeshData>& out) = 0;
            virtual FullVertexCacheInfo GenerateFullVertexCache() = 0;
            virtual rendering::BufferID GetIndexBuffer() = 0;
            virtual rendering::BufferID GetVertexBuffer() = 0;
        };

        virtual Atlas* CreateAtlas(AtlasType type, int glyphSize)=0;
        virtual VertexCache* CreateVertexCache()=0;

        static Font* CreateFromFile(const char8_t* u8filename);
        static Font* CreateFromMappedMemory(const u8* fontDataPtr, size_t size);
        
        virtual ~Font() = default;

       virtual u32 GetNumberOfGlyphs() = 0;

       virtual std::vector<u8> GeneratePreviewRGBA(int width, int height) = 0;
    };
}
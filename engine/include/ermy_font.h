#pragma once
#include "ermy_rendering.h"
#include <glm/glm.hpp>

namespace ermy
{
    class Font
    {
    public:
        enum class AtlasType
        {
            Grayscale,
            SDF,
            MSDFT
        };

        struct SpriteData
        {
            u16 u0;
            u16 v0;
            u16 u1;
            u16 v1;

            glm::vec4 TexCoords()
            {
                constexpr float v = 1.0f / 65535.0f;

                return glm::vec4(static_cast<float>(u0) * v,
                    static_cast<float>(v0) * v,
                    static_cast<float>(u1) * v,
                    static_cast<float>(v1) * v);
            }
        };
        static Font* CreateFromFile(const char8_t* u8filename, AtlasType atlasType = AtlasType::Grayscale);
        
        virtual ~Font() = default;
        virtual rendering::TextureID GetAtlas() = 0;

        virtual void FillTexCoords(const char8_t* u8string, std::vector<SpriteData>& out) = 0;
    };
}
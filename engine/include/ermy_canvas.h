#pragma once
#include "ermy_api.h"
#include "ermy_shader.h"
#include "ermy_rendering.h"
namespace ermy
{
    namespace canvas
    {
        void SetClearColor(float r, float g, float b, float a);
        //void DrawDedicatedSprite(float x, float y, float w, float h, float a);
		rendering::TextureID GetWhiteTextureID(); 
        void DrawDedicatedSprite(rendering::TextureID texture, float x, float y, float w, float h, float a, u32 packedColor, float u0, float v0, float u1, float v1);
    
        inline void DrawDedicatedSprite(rendering::TextureID texture, float x, float y, float w, float h, float a = 0.0f, u32 packedColor = 0xFFFFFFFF)
        {
			DrawDedicatedSprite(texture, x, y, w, h, a, packedColor, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        inline void DrawDedicatedSprite(float x, float y, float w, float h, float a = 0.0f, u32 packedColor = 0xFFFFFFFF)
        {
			DrawDedicatedSprite(GetWhiteTextureID(), x, y, w, h, a, packedColor);
        }
    }
}


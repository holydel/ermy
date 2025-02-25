#pragma once
#include "ermy_api.h"
#include "ermy_shader.h"
#include "ermy_rendering.h"
namespace ermy
{
    namespace canvas
    {
        void SetClearColor(float r, float g, float b, float a);
        void DrawDedicatedSprite(float x, float y, float w, float h, float a);

        void DrawDedicatedSprite(rendering::TextureID texture, float x, float y, float w, float h, float a);
    }
}


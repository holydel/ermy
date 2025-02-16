#include "canvas_interface.h"
#include <ermy_shader_internal.h>
#include <cassert>

using namespace ermy;
using namespace ermy::rendering;

float canvas_interface::BgColor[4] = {1.0f,1.0f,1.0f,1.0f};
CommandList* gCanvasCL = nullptr;
PSOID gDedicatedSpritePSO;

void ermy::canvas::SetClearColor(float r, float g, float b, float a)
{
    canvas_interface::BgColor[0] = r;
    canvas_interface::BgColor[1] = g;
    canvas_interface::BgColor[2] = b;
    canvas_interface::BgColor[3] = a;
}

void canvas_interface::SetCommandList(ermy::rendering::CommandList* cl)
{
    gCanvasCL = cl;
}

void ermy::canvas::DrawDedicatedSprite(float x, float y, float w, float h, float a)
{
    assert(gCanvasCL);

    gCanvasCL->SetPSO(gDedicatedSpritePSO);

    struct SpriteInfo
    {
        float x, y, w, h, u0, v0, u1, v1;
        float angle;
        u32 packedColor;
    };

    SpriteInfo sinfo = { x,y,w,h,0.0f,0.0f,1.0f,1.0f,a,0xFFFFFFFFu };

    gCanvasCL->SetRootConstants(&sinfo, sizeof(sinfo));
    gCanvasCL->Draw(4);
}

void canvas_interface::Initialize()
{
    PSODesc desc;
    desc.shaders.push_back(shader_internal::dedicatedSpriteVS());
    desc.shaders.push_back(shader_internal::dedicatedSpriteFS());
    desc.topology = PrimitiveTopology::TriangleStrip;
    desc.rootConstantRanges[(int)ShaderStage::Vertex] = { 0,10 }; //float2 pos, float2 size, float2 uv0, float2 uv1, uint packedColor, float angle

    gDedicatedSpritePSO = CreatePSO(desc);
}

void canvas_interface::Shutdown()
{

}
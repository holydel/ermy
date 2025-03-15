#include "canvas_interface.h"
#include <ermy_shader_internal.h>
#include <cassert>
#include "../rendering/framegraph.h"

using namespace ermy;
using namespace ermy::rendering;

float canvas_interface::BgColor[4] = {1.0f,1.0f,1.0f,1.0f};
CommandList* gCanvasCL = nullptr;
PSOID gDedicatedSpritePSO;
TextureID gWhiteTexture;
ermy::u64 gCanvasDescriptorSet;

struct SpriteInfo
{
    float x, y, w, h, u0, v0, u1, v1;
    float angle;
    u32 packedColor;
};

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
    //cl->SetDescriptorSet(0, gCanvasDescriptorSet);
}

ermy::rendering::TextureID ermy::canvas::GetWhiteTextureID()
{
	return gWhiteTexture;
}

extern BufferID gFrameConstants;

void ermy::canvas::DrawDedicatedSprite(rendering::TextureID texture, float x, float y, float w, float h, float a, u32 packedColor, float u0, float v0, float u1, float v1)
{
    assert(gCanvasCL);

    gCanvasCL->SetPSO(gDedicatedSpritePSO);

    SpriteInfo sinfo = { x,y,w,h,u0,v0,u1,v1,a,packedColor };

    gCanvasCL->SetDescriptorSet(0, gCanvasDescriptorSet);
	gCanvasCL->SetDescriptorSet(1, rendering::GetTextureDescriptor(texture));
    gCanvasCL->SetRootConstants(&sinfo, sizeof(sinfo));
    gCanvasCL->Draw(4);
}

void canvas_interface::Initialize()
{
    PSODesc desc;
    desc.SetShaderStage(shader_internal::dedicatedSpriteVS());
    desc.SetShaderStage(shader_internal::dedicatedSpriteFS());
    desc.domain = PSODomain::Canvas;
	desc.uniforms.push_back(ShaderUniformType::Texture2D);
    desc.topology = PrimitiveTopology::TriangleStrip;
    desc.rootConstantRanges[(int)ShaderStage::Vertex] = { 0,sizeof(SpriteInfo)}; //float2 pos, float2 size, float2 uv0, float2 uv1, uint packedColor, float angle
	desc.debugName = "DedicatedSprite";
    gDedicatedSpritePSO = CreatePSO(desc);

	TextureDesc whiteTexDesc;
	whiteTexDesc.debugName = "WhiteTexture";
	whiteTexDesc.width = 1;
	whiteTexDesc.height = 1;
	whiteTexDesc.depth = 1;
	whiteTexDesc.numLayers = 1;
	whiteTexDesc.numMips = 1;
	whiteTexDesc.isSparse = false;
	whiteTexDesc.texelSourceFormat = Format::RGBA8_UNORM;
	whiteTexDesc.dataSize = 4;
	u8 whiteData[4] = { 255,255,255,255 };
	whiteTexDesc.pixelsData = whiteData;

	gWhiteTexture = CreateDedicatedTexture(whiteTexDesc);

    
    ermy::rendering::DescriptorSetDesc dsDesc;
    dsDesc.AddBindingUniformBuffer(gFrameConstants);
    gCanvasDescriptorSet = ermy::rendering::CreateDescriptorSet(rendering::PSODomain::Canvas, dsDesc);
}

void canvas_interface::Shutdown()
{

}
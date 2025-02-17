#include <assets/texture_asset.h>
#include <ermy_log.h>
#include <compressonator.h>
#include <imgui.h>
#include <ermy_utils.h>
#include "editor_shader_internal.h"
#include "preview_renderer.h"

using namespace ermy;

struct TextureLivePreviewParams2D
{
	glm::vec2 uv0;
	glm::vec2 uv1;
};

struct TextureLivePreviewParamsCube
{
	glm::vec4 dir_tanfov;	
};

class TextureRenderPreview
{
	TextureRenderPreview();
	~TextureRenderPreview();

	rendering::PSOID fullscreenEmpty;
	rendering::PSOID fullscreen2D;
	rendering::PSOID fullscreenCube;

	rendering::PSOID fullscreen2DStatic;
	rendering::PSOID fullscreenCubeStatic;
public:
	static TextureRenderPreview& Instance()
	{
		static TextureRenderPreview instance;
		return instance;
	}

	void BindPSO(rendering::CommandList& cl, rendering::TextureType texType, bool isStatic = false)
	{
		if (texType == rendering::TextureType::Tex2D)
		{
			if(isStatic)
				cl.SetPSO(fullscreen2DStatic);
			else
				cl.SetPSO(fullscreen2D);
			return;
		}
		if (texType == rendering::TextureType::TexCube)
		{
			if (isStatic)
				cl.SetPSO(fullscreenCubeStatic);
			else
				cl.SetPSO(fullscreenCube);
			return;
		}

		cl.SetPSO(fullscreenEmpty);
	}
};

TextureRenderPreview::TextureRenderPreview()
{
	auto RTT = PreviewRenderer::Instance().GetRTT();
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	//live preview PSD
	{
		rendering::PSODesc desc;
		desc.shaders.push_back(ermy::shader_internal::fullscreenVS());
		desc.shaders.push_back(ermy::shader_internal::fullscreenFSEmpty());
		desc.specificRenderPass = RTT;
		fullscreenEmpty = rendering::CreatePSO(desc);
	}
	
	{
		rendering::PSODesc desc;
		desc.shaders.push_back(ermy::shader_internal::fullscreenVS());
		desc.shaders.push_back(ermy::shader_internal::fullscreenFS2D());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		//desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = RTT;
		desc.rootConstantRanges[(int)ShaderStage::Fragment] = { 0,sizeof(TextureLivePreviewParams2D)};
		fullscreen2D = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.shaders.push_back(ermy::shader_internal::fullscreenVS());
		desc.shaders.push_back(ermy::shader_internal::fullscreenFSCubemap());
		//desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = RTT;
		desc.rootConstantRanges[(int)ShaderStage::Fragment] = { 0,sizeof(TextureLivePreviewParamsCube) };
		fullscreenCube = rendering::CreatePSO(desc);
	}

	//static preview
	{
		rendering::PSODesc desc;
		desc.shaders.push_back(ermy::shader_internal::fullscreenVS());
		desc.shaders.push_back(ermy::shader_internal::fullscreenFS2DStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		//desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = staticRTT;
		fullscreen2DStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.shaders.push_back(ermy::shader_internal::fullscreenVS());
		desc.shaders.push_back(ermy::shader_internal::fullscreenFSCubemapStatic());
		//desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = staticRTT;
		fullscreenCubeStatic = rendering::CreatePSO(desc);
	}
}

TextureRenderPreview::~TextureRenderPreview()
{

}

TextureAsset::TextureAsset()
{
   
}

TextureAsset::~TextureAsset()
{

}
void TextureAsset::ResetView()
{
	previewZoom = 1.0f;
	previewDX = previewDY = oldPreviewDX = oldPreviewDY = 0.0f;
	isPreviewDragging = false;
}

void TextureAsset::MouseZoom(float value)
{
	previewZoom /= value;
}
void TextureAsset::MouseDown(float normalizedX, float normalizedY)
{
	if (!isPreviewDragging)
	{
		isPreviewDragging = true;
		oldPreviewDX = previewDX;
		oldPreviewDY = previewDY;
	}
	
}
void TextureAsset::MouseUp()
{
	isPreviewDragging = false;
}

void TextureAsset::MouseMove(float normalizedDeltaX, float normalizedDeltaY)
{
	if (isPreviewDragging)
	{
		previewDX = oldPreviewDX - (normalizedDeltaX * previewZoom);
		previewDY = oldPreviewDY - (normalizedDeltaY * previewZoom);
	}
}

void TextureAsset::DrawPreview()
{
	ImGui::Text("Width: %d Height: %d",width,height);
	ImGui::Text("Datasize: %s",ermy_utils::string::humanReadableFileSize(dataSize).c_str());

	ImGui::Checkbox("IsStatic", &isStaticPreview);
}

void TextureAsset::RegeneratePreview()
{
	if (isCubemap)
	{
		if (numLayers > 6)
			texType = rendering::TextureType::TexArrayCube;
		else
			texType = rendering::TextureType::TexCube;
	}
	else
	{
		if(depth > 1)
			texType = rendering::TextureType::Tex3D;
		else
		{
			if (numLayers > 1)
				texType = rendering::TextureType::TexArray2D;
		}
	}

	ermy::rendering::TextureDesc descLive;
	descLive.width = width;
	descLive.height = height;
	descLive.depth = depth;
	descLive.isCubemap = isCubemap;
	descLive.numLayers = numLayers;
	descLive.numMips = numMips;
	descLive.pixelsData = data;
	descLive.isSparse = false;
	descLive.texelFormat = texelFormat;
	descLive.dataSize = dataSize;

	previewTextureLive = ermy::rendering::CreateDedicatedTexture(descLive);

	ermy::rendering::TextureDesc descStatic;
	descStatic.width = 128;
	descStatic.height = 128;
	descStatic.depth = 1;
	descStatic.isCubemap = false;
	descStatic.numLayers = 1;
	descStatic.numMips = 1;
	descStatic.isSparse = false;
	descStatic.texelFormat = ermy::rendering::Format::RGBA8_UNORM;

	descStatic.pixelsData = nullptr;
	descStatic.dataSize = 0;

	previewTextureStatic = ermy::rendering::CreateDedicatedTexture(descStatic);
	assetPreviewTexLive = ermy::rendering::GetTextureDescriptor(previewTextureLive);
	assetPreviewTexStatic = ermy::rendering::GetTextureDescriptor(previewTextureStatic);

	PreviewRenderer::Instance().EnqueueStaticPreviewGeneration(this);
}

void TextureAsset::RenderStaticPreview(ermy::rendering::CommandList& cl)
{
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	cl.BeginRenderPass(staticRTT);
	TextureRenderPreview::Instance().BindPSO(cl, texType, true);
	cl.SetDescriptorSet(0, assetPreviewTexLive);
	cl.Draw(3);
	cl.EndRenderPass();

	auto staticRTTTex = PreviewRenderer::Instance().GetStaticTexture();
	cl.BlitTexture(staticRTTTex, previewTextureStatic);
	int a = 42;
}

void TextureAsset::RenderPreview(ermy::rendering::CommandList& cl)
{
	TextureRenderPreview::Instance().BindPSO(cl, texType,isStaticPreview);
	{
		if (texType == rendering::TextureType::Tex2D)
		{
			TextureLivePreviewParams2D pass;
			float aspect = (float)width / (float)height;

			float baseU = 1.0f;
			float baseV = 1.0f;

			if (aspect < 1.0f)
			{
				baseU /= aspect;
			}
			else
			{
				baseV *= aspect;
			}

			float baseU0 = 0.5f - (0.5f * baseU * previewZoom) + previewDX;
			float baseU1 = 0.5f + (0.5f * baseU * previewZoom) + previewDX;
			float baseV0 = 0.5f - (0.5f * baseV * previewZoom) + previewDY;
			float baseV1 = 0.5f + (0.5f * baseV * previewZoom) + previewDY;

			pass.uv0 = glm::vec2(baseU0, baseV0);
			pass.uv1 = glm::vec2(baseU1, baseV1);

			if(!isStaticPreview)
			cl.SetRootConstant(pass,ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTexLive);
		}

		if (texType == rendering::TextureType::TexCube)
		{
			TextureLivePreviewParamsCube pass;

			float pitch = previewDY * 3.1415f;
			float yaw = previewDX * 3.1415f;

			float dirx = std::cos(pitch) * std::cos(yaw); // Forward (X)
			float diry = std::sin(pitch);                 // Up (Y)
			float dirz = std::cos(pitch) * std::sin(yaw); // Right (Z)

			pass.dir_tanfov = glm::vec4(dirx, diry, dirz, previewZoom);

			if(!isStaticPreview)
			cl.SetRootConstant(pass, ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTexLive);
		}
	}
	
	cl.Draw(3);
}

//std::vector<std::string>  TextureAsset::Initialize()
//{
//    std::string supported_formats;
//    getattribute("input_format_list", supported_formats);
//    ERMY_LOG("Supported image formats: %s\n", supported_formats.c_str());
//    std::cout << "Supported image formats: " << supported_formats << std::endl;
//
//    std::vector<std::string> result;
//    return result;
//}
//
//void TextureAsset::Shutdown()
//{
//
//}
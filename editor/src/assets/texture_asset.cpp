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
public:
	static TextureRenderPreview& Instance()
	{
		static TextureRenderPreview instance;
		return instance;
	}

	void BindPSO(rendering::CommandList& cl, rendering::ShaderUniformType utype)
	{
		if (utype == rendering::ShaderUniformType::Texture2D)
		{
			cl.SetPSO(fullscreen2D);
			return;
		}
		if (utype == rendering::ShaderUniformType::TextureCube)
		{
			cl.SetPSO(fullscreenCube);
			return;
		}

		cl.SetPSO(fullscreenEmpty);
	}
};

TextureRenderPreview::TextureRenderPreview()
{
	auto RTT = PreviewRenderer::Instance().GetRTT();

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

rendering::ShaderUniformType gUtype = rendering::ShaderUniformType::TextureVolume;

void TextureAsset::DrawPreview()
{
	ImGui::Text("Width: %d Height: %d",width,height);
	ImGui::Text("Datasize: %s",ermy_utils::string::humanReadableFileSize(dataSize).c_str());
	if (ImGui::Button("2D"))
		gUtype = rendering::ShaderUniformType::Texture2D;
	

	ImGui::SameLine();
	if(ImGui::Button("Array"))
		gUtype = rendering::ShaderUniformType::Texture2DArray;

	ImGui::SameLine();
	if(ImGui::Button("Cube"))
		gUtype = rendering::ShaderUniformType::TextureCube;

	ImGui::SameLine();
	if(ImGui::Button("3D"))
		gUtype = rendering::ShaderUniformType::TextureVolume;
}

void TextureAsset::RegeneratePreview()
{
	ermy::rendering::TextureDesc desc;
	desc.width = width;
	desc.height = height;
	desc.depth = depth;
	desc.isCubemap = isCubemap;
	desc.numLayers = numLayers;
	desc.numMips = numMips;
	desc.pixelsData = data;
	desc.isSparse = false;
	desc.texelFormat = texelFormat;
	desc.dataSize = dataSize;

	previewTexture = ermy::rendering::CreateDedicatedTexture(desc);
	assetPreviewTex = ermy::rendering::GetTextureDescriptor(previewTexture);
}

void TextureAsset::RenderPreview(ermy::rendering::CommandList& cl)
{
	TextureRenderPreview::Instance().BindPSO(cl, gUtype);
	{
		if (gUtype == rendering::ShaderUniformType::Texture2D)
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

			cl.SetRootConstant(pass,ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTex);
		}

		if (gUtype == rendering::ShaderUniformType::TextureCube)
		{
			TextureLivePreviewParamsCube pass;

			float pitch = previewDY * 3.1415f;
			float yaw = previewDX * 3.1415f;

			float dirx = std::cos(pitch) * std::cos(yaw); // Forward (X)
			float diry = std::sin(pitch);                 // Up (Y)
			float dirz = std::cos(pitch) * std::sin(yaw); // Right (Z)

			pass.dir_tanfov = glm::vec4(dirx, diry, dirz, previewZoom);
			cl.SetRootConstant(pass, ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTex);
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
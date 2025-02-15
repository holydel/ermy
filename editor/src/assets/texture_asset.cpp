#include <assets/texture_asset.h>
#include <ermy_log.h>
#include <compressonator.h>
#include <imgui.h>
#include <ermy_utils.h>
#include "editor_shader_internal.h"
#include "preview_renderer.h"

using namespace ermy;

class TextureRenderPreview
{
	TextureRenderPreview();
	~TextureRenderPreview();

	rendering::PSOID fullscreenEmpty;
	rendering::PSOID fullscreen2D;
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
		desc.specificRenderPass = RTT;
		fullscreen2D = rendering::CreatePSO(desc);
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

	previewTexture = ermy::rendering::CreateDedicatedTexture(desc);
	assetPreviewTex = ermy::rendering::GetTextureDescriptor(previewTexture);
}

void TextureAsset::RenderPreview(ermy::rendering::CommandList& cl)
{
	TextureRenderPreview::Instance().BindPSO(cl, gUtype);
	if(gUtype == rendering::ShaderUniformType::Texture2D)
	cl.SetDescriptorSet(0, assetPreviewTex);
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
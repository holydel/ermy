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
	int arrayLevel;
};

struct TextureLivePreviewParamsCube
{
	glm::vec4 dir_tanfov;	
	int arrayLevel;
};

class TextureRenderPreview
{
	TextureRenderPreview();
	~TextureRenderPreview();

	rendering::PSOID fullscreenEmpty;
	rendering::PSOID fullscreen2D;
	rendering::PSOID fullscreenCube;
	rendering::PSOID fullscreen2DArray;
	rendering::PSOID fullscreenCubeArray;
	rendering::PSOID fullscreen3D;

	rendering::PSOID fullscreen2DStatic;
	rendering::PSOID fullscreenCubeStatic;
	rendering::PSOID fullscreen2DArrayStatic;
	rendering::PSOID fullscreenCubeArrayStatic;
	rendering::PSOID fullscreen3DStatic;

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
		if (texType == rendering::TextureType::TexArray2D)
		{
			if (isStatic)
				cl.SetPSO(fullscreen2DArrayStatic);
			else
				cl.SetPSO(fullscreen2DArray);
			return;
		}
		if (texType == rendering::TextureType::TexArrayCube)
		{
			if (isStatic)
				cl.SetPSO(fullscreenCubeArrayStatic);
			else
				cl.SetPSO(fullscreenCubeArray);
			return;
		}
		if (texType == rendering::TextureType::Tex3D)
		{
			if (isStatic)
				cl.SetPSO(fullscreen3DStatic);
			else
				cl.SetPSO(fullscreen3D);
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
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSEmpty());
		desc.specificRenderPass = RTT;
		fullscreenEmpty = rendering::CreatePSO(desc);
	}
	
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2D());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		//desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParams2D));
		fullscreen2D = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemap());
		//desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParamsCube));
		fullscreenCube = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2DArray());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2DArray);
		//desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParams2D));
		fullscreen2DArray = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemapArray());
		//desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCubeArray);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParamsCube));
		fullscreenCubeArray = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		//desc.SetShaderStage(ermy::shader_internal::fullscreenFSVolumetric());
		//desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureVolume);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParamsCube));
		//fullscreen3D = rendering::CreatePSO(desc);
	}

	//static preview
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2DStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.specificRenderPass = staticRTT;
		fullscreen2DStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemapStatic());
		//desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = staticRTT;
		fullscreenCubeStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2DArrayStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2DArray);
		desc.specificRenderPass = staticRTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(int));
		fullscreen2DArrayStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemapArrayStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2DArray);
		desc.specificRenderPass = staticRTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(int));
		fullscreenCubeArrayStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSVolumetricStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureVolume);
		desc.specificRenderPass = staticRTT;
		fullscreen3DStatic= rendering::CreatePSO(desc);
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
	if (texType == rendering::TextureType::TexArrayCube || texType == rendering::TextureType::TexArray2D)
	{
		ImGui::SliderInt("Layer", &currentArrayLevel, 0, (isCubemap ? numLayers / 6 : numLayers) - 1);
	}
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

	if (texType == rendering::TextureType::TexArrayCube || texType == rendering::TextureType::TexArray2D)
	{
		int actualLayers = isCubemap ? numLayers / 6 : numLayers;

		cl.SetRootConstant(actualLayers, ShaderStage::Fragment);
	}

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
		if (texType == rendering::TextureType::Tex2D || texType == rendering::TextureType::TexArray2D)
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
			pass.arrayLevel = currentArrayLevel;
			if(!isStaticPreview)
			cl.SetRootConstant(pass,ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTexLive);
		}

		if (texType == rendering::TextureType::TexCube || texType == rendering::TextureType::TexArrayCube || texType == rendering::TextureType::Tex3D)
		{
			TextureLivePreviewParamsCube pass;

			float pitch = previewDY * 3.1415f;
			float yaw = previewDX * 3.1415f;

			float dirx = std::cos(pitch) * std::cos(yaw); // Forward (X)
			float diry = std::sin(pitch);                 // Up (Y)
			float dirz = std::cos(pitch) * std::sin(yaw); // Right (Z)

			pass.dir_tanfov = glm::vec4(dirx, diry, dirz, previewZoom);
			pass.arrayLevel = currentArrayLevel;
			if(!isStaticPreview)
			cl.SetRootConstant(pass, ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTexLive);
		}
	}
	
	cl.Draw(3);
}
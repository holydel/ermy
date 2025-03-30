#include <assets/texture_asset.h>
#include <ermy_log.h>
#include <imgui.h>
#include <ermy_utils.h>
#include "editor_shader_internal.h"
#include "preview_renderer.h"
#include <editor_file_utils.h>

#ifdef NDEBUG
#pragma comment(lib,"Compressonator_MD.lib")
#else
#pragma comment(lib,"Compressonator_MDd.lib")
#endif

using namespace ermy;

struct TextureLivePreviewParams2D
{
	glm::vec2 uv0;
	glm::vec2 uv1;
	int arrayLevel;
	int mipLevel;
};

struct TextureLivePreviewParamsCube
{
	glm::vec4 dir_tanfov;	
	int arrayLevel;
	int mipLevel;
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
		desc.debugName = "FullscreenEmpty";
		fullscreenEmpty = rendering::CreatePSO(desc);
	}
	
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2D());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParams2D));
		desc.debugName = "Fullscreen2D";
		fullscreen2D = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemap());
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParamsCube));
		desc.debugName = "FullscreenCube";
		fullscreenCube = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2DArray());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2DArray);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParams2D));
		desc.debugName = "Fullscreen2DArray";
		fullscreen2DArray = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemapArray());
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCubeArray);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParamsCube));
		desc.debugName = "FullscreenCubeArray";
		fullscreenCubeArray = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSVolumetric());
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureVolume);
		desc.specificRenderPass = RTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(TextureLivePreviewParamsCube));
		desc.debugName = "Fullscreen3D";
		fullscreen3D = rendering::CreatePSO(desc);
	}

	//static preview
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2DStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.specificRenderPass = staticRTT;
		desc.debugName = "Fullscreen2DStatic";
		fullscreen2DStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemapStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureCube);
		desc.specificRenderPass = staticRTT;
		desc.debugName = "FullscreenCubeStatic";
		fullscreenCubeStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFS2DArrayStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2DArray);
		desc.specificRenderPass = staticRTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(int));
		desc.debugName = "Fullscreen2DArrayStatic";
		fullscreen2DArrayStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSCubemapArrayStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2DArray);
		desc.specificRenderPass = staticRTT;
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(int));
		desc.debugName = "FullscreenCubeArrayStatic";
		fullscreenCubeArrayStatic = rendering::CreatePSO(desc);
	}

	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSVolumetricStatic());
		desc.uniforms.push_back(rendering::ShaderUniformType::TextureVolume);
		desc.specificRenderPass = staticRTT;
		desc.debugName = "Fullscreen3DStatic";
		fullscreen3DStatic= rendering::CreatePSO(desc);
	}
}

TextureRenderPreview::~TextureRenderPreview()
{

}

TextureAsset::TextureAsset()
{
	//CMP_InitializeBCLibrary(&mipSet);
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

void TextureAsset::CompressTexture()
{
	//CompressonatorLib::Instance().CompressMips(sourceMipSet, targetMipSet, texelSourceFormat, texelTargetFormat);
}

void TextureAsset::RegenerateMips()
{
	printf("foo");
}

ChannelFormat GetChannelFormatFromErmyFormat(ermy::rendering::Format format)
{
	using namespace ermy::rendering;
	
	switch(format)
	{
		case Format::R8_UNORM:
		case Format::RG8_UNORM:
		case Format::RGBA8_UNORM:
		case Format::RGBA8_SRGB:
			return CF_8bit;
		case Format::R16F:
		case Format::RG16F:
		case Format::RGBA16F:
			return CF_Float16;
		case Format::RGBA16_UNORM:
		case Format::RGBA16_NORM:
			return CF_16bit;
		case Format::R32F:
		case Format::RG32F:
		case Format::RGBA32F:
			return CF_32bit;
		case Format::BC1:
		case Format::BC5:			
		case Format::BC6:		
		case Format::BC6_SF:			
		case Format::BC7:			
			return CF_Compressed;
		case Format::D16_UNORM:
		case Format::D24_UNORM_S8_UINT:
		case Format::D32F:
			assert(false);
		default:
			return CF_8bit;
	}
}

void TextureAsset::SetSourceData(const ermy::u8* data, ermy::u32 dataSize)
{
	rawData.resize(dataSize);
	memcpy(rawData.data(), data, dataSize);
	RegenerateLivePreview();
}

void TextureAsset::MouseZoom(float value)
{
	previewZoom /= value;
}
void TextureAsset::MouseDown(float normalizedX, float normalizedY, int button)
{
	if (button == 0)
	{
		if (!isPreviewDragging)
		{
			isPreviewDragging = true;
			oldPreviewDX = previewDX;
			oldPreviewDY = previewDY;
		}
	}
}
void TextureAsset::MouseUp(int button)
{
	if(button == 0)
		isPreviewDragging = false;
}

void TextureAsset::MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button)
{
	if (button == 0)
	{
		if (isPreviewDragging)
		{
			previewDX = oldPreviewDX - (normalizedDeltaX * previewZoom);
			previewDY = oldPreviewDY - (normalizedDeltaY * previewZoom);
		}
	}
}

void TextureAsset::DrawPreview()
{
	ImGui::Text("Width: %d Height: %d",width,height);
	ImGui::Text("Datasize: %s",ermy_utils::string::humanReadableFileSize(rawData.size()).c_str());
	
	ImGui::Checkbox("sRGB", &isSRGBSpace);
	ImGui::SameLine();
	ImGui::Checkbox("Regen MIPs", &regenerateMips);

	if (regenerateMips && numMips == 1)
	{
		RegenerateMips();
	}

	if (texType == rendering::TextureType::TexArrayCube || texType == rendering::TextureType::TexArray2D)
	{
		ImGui::SliderInt("Layer", &currentArrayLevel, 0, (isCubemap ? numLayers / 6 : numLayers) - 1);
	}

	if (numMips > 1)
	{
		ImGui::SliderInt("Mip", &currentMip, 0,numMips-1);
	}

	int curPurpose = (int)texturePurpose;
	ImGui::Combo("Purpose:", &curPurpose, TexturePurposeNames,std::size(TexturePurposeNames));
	
	if (curPurpose != (int)texturePurpose)
	{
		texturePurpose = (TexturePurpose)curPurpose;
		UpdateTextureSettings();
	}

	if (texturePurpose == TexturePurpose::TP_SOURCE)
	{
		int curCompression = (int)textureCompression;
		ImGui::Combo("Compression:", &curCompression, TextureCompressionNames, std::size(TextureCompressionNames));

		if (curCompression != (int)textureCompression)
		{
			textureCompression = (TextureCompression)curCompression;
			UpdateTextureSettings();
		}		
	}
}


void TextureAsset::UpdateTextureSettings()
{
	switch (texturePurpose)
	{
	case TexturePurpose::TP_ALBEDO:
		isSRGBSpace = true;
		regenerateMips = true;
		break;
	case TexturePurpose::TP_HDR:
		isSRGBSpace = false;
		regenerateMips = true;
		break;
	case TexturePurpose::TP_NORMALMAP:
		isSRGBSpace = false;
		regenerateMips = true;
		break;
	case TexturePurpose::TP_UI:
		isSRGBSpace = true;
		regenerateMips = false;
		break;
	case TexturePurpose::TP_UTILITY:
		isSRGBSpace = false;
		regenerateMips = true;
		break;
	}
}

void TextureAsset::LoadFromCachedRaw(std::ifstream& file, const std::filesystem::path& path)
{
	width = readBinary<u32>(file);
	height = readBinary<u32>(file);
	depth = readBinary<u32>(file);
	numLayers = readBinary<u32>(file);
	numMips = readBinary<u32>(file);
	isCubemap = readBinary<bool>(file);
	isSparse = readBinary<bool>(file);
	texelSourceFormat = readBinary<rendering::Format>(file);
	texelTargetFormat = readBinary<rendering::Format>(file);
	texturePurpose = readBinary<TexturePurpose>(file);
	texType = readBinary<rendering::TextureType>(file);
	textureCompression = readBinary<TextureCompression>(file);
	isSRGBSpace = readBinary<bool>(file);
	regenerateMips = readBinary<bool>(file);

	rawData = readVector<u8>(file);
	RegenerateLivePreview();

}

void TextureAsset::SaveToCachedRaw(std::ofstream& file)
{
	writeBinary(file, width);
	writeBinary(file, height);
	writeBinary(file, depth);
	writeBinary(file, numLayers);
	writeBinary(file, numMips);
	writeBinary(file, isCubemap);
	writeBinary(file, isSparse);
	writeBinary(file, texelSourceFormat);
	writeBinary(file, texelTargetFormat);
	writeBinary(file, texturePurpose);
	writeBinary(file, texType);
	writeBinary(file, textureCompression);
	writeBinary(file, isSRGBSpace);
	writeBinary(file, regenerateMips);

	writeVector(file, rawData);
}

void TextureAsset::RegenerateLivePreview()
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
	descLive.pixelsData = rawData.data();
	descLive.isSparse = false;
	descLive.texelSourceFormat = texelSourceFormat;
	descLive.dataSize = rawData.size();

	previewTextureLive = ermy::rendering::CreateDedicatedTexture(descLive);
	assetPreviewTexLive = ermy::rendering::GetTextureDescriptor(previewTextureLive);
}

std::vector<ermy::u8> TextureAsset::GetStaticPreviewData()
{
	return PreviewRenderer::Instance().GetPreviewDataBC1([&](ermy::rendering::CommandList& cl)
	{
		RenderStaticPreview(cl);
	});
}

void TextureAsset::RenderStaticPreview(ermy::rendering::CommandList& cl)
{	
	TextureRenderPreview::Instance().BindPSO(cl, texType, true);
	cl.SetDescriptorSet(0, assetPreviewTexLive);

	if (texType == rendering::TextureType::TexArrayCube || texType == rendering::TextureType::TexArray2D)
	{
		int actualLayers = isCubemap ? numLayers / 6 : numLayers;

		cl.SetRootConstant(actualLayers, ShaderStage::Fragment);
	}

	cl.Draw(3);	
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
			pass.mipLevel = currentMip;
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
			pass.mipLevel = currentMip;

			if(!isStaticPreview)
			cl.SetRootConstant(pass, ShaderStage::Fragment);
			cl.SetDescriptorSet(0, assetPreviewTexLive);
		}
	}
	
	cl.Draw(3);
}

CMP_FORMAT CMPFormatFromErmyFormat(ermy::rendering::Format format)
{
	using namespace rendering;

	switch (format) {
		// Uncompressed Byte Formats (0x0nn0)
	case Format::RGBA8_UNORM: return CMP_FORMAT_RGBA_8888;
	case Format::ARGB8_UNORM: return CMP_FORMAT_ARGB_8888;
	case Format::RGBA8_SRGB: return CMP_FORMAT_RGBA_8888; // Compressonator doesn't distinguish sRGB here; handle in pipeline
	case Format::BGRA8_UNORM: return CMP_FORMAT_BGRA_8888;
	case Format::R8_UNORM: return CMP_FORMAT_R_8;
	case Format::RG8_UNORM: return CMP_FORMAT_RG_8;
	case Format::RGBA8_UINT: return CMP_FORMAT_RGBA_8888; // UINT not directly supported, treat as UNORM
	case Format::RGBA16_UNORM: return CMP_FORMAT_RGBA_16;
	case Format::RGBA16_NORM: return CMP_FORMAT_RGBA_16; // Signed not distinguished, treat as UNORM
	case Format::RG16_UNORM: return CMP_FORMAT_RG_16;
	case Format::R16_UNORM: return CMP_FORMAT_R_16;
	case Format::RGBA16_UINT: return CMP_FORMAT_RGBA_16; // UINT not directly supported, treat as UNORM
	case Format::RG16_UINT: return CMP_FORMAT_RG_16;     // UINT not directly supported, treat as UNORM
	case Format::R16_UINT: return CMP_FORMAT_R_16;       // UINT not directly supported, treat as UNORM

		// Uncompressed Float Formats (0x1nn0)
	case Format::RGBA16F: return CMP_FORMAT_RGBA_16F;
	case Format::RG16F: return CMP_FORMAT_RG_16F;
	case Format::R16F: return CMP_FORMAT_R_16F;
	case Format::R32F: return CMP_FORMAT_R_32F;
	case Format::RG32F: return CMP_FORMAT_RG_32F;
	case Format::RGB32F: return CMP_FORMAT_RGB_32F;
	case Format::RGBA32F: return CMP_FORMAT_RGBA_32F;

		// Compressed Formats (0xSnn1..0xSnnF)
	case Format::BC1: return CMP_FORMAT_BC1; // Assumed UNORM, use TC_BC1_SRGB for sRGB in TextureCompression
	case Format::BC2: return CMP_FORMAT_BC2;
	case Format::BC3: return CMP_FORMAT_BC3;
	case Format::BC4: return CMP_FORMAT_BC4;
	case Format::BC5: return CMP_FORMAT_BC5;
	case Format::BC6: return CMP_FORMAT_BC6H; // Assuming BC6H UF16 (unsigned float)
	case Format::BC6_SF: return CMP_FORMAT_BC6H_SF; // Assuming BC6H UF16 (unsigned float)
	case Format::BC7: return CMP_FORMAT_BC7;

		// Depth Formats (not directly supported in CMP_FORMAT, map to closest or unknown)
	case Format::D32F: return CMP_FORMAT_R_32F; // Treat as single-channel float
	case Format::D16_UNORM: return CMP_FORMAT_R_16; // Treat as single-channel UNORM
	case Format::D24_UNORM_S8_UINT: return CMP_FORMAT_Unknown; // No direct equivalent

		// Default case
	default:
		ERMY_ERROR("Unsupported ermy::rendering::Format: %d", static_cast<int>(format));
		return CMP_FORMAT_Unknown;
	}
}

void TextureAsset::Save(pugi::xml_node& node)
{
	AssetData::Save(node);

	node.append_attribute("isSRGBSpace").set_value(isSRGBSpace);
	node.append_attribute("regenerateMips").set_value(regenerateMips);
	node.append_attribute("texturePurpose").set_value(static_cast<int>(texturePurpose));
	node.append_attribute("textureCompression").set_value(static_cast<int>(textureCompression));
}

void TextureAsset::Load(pugi::xml_node& node)	
{
	AssetData::Load(node);

	isSRGBSpace = node.attribute("isSRGBSpace").as_bool();
	regenerateMips = node.attribute("regenerateMips").as_bool();
	texturePurpose = static_cast<TexturePurpose>(node.attribute("texturePurpose").as_int());
	textureCompression = static_cast<TextureCompression>(node.attribute("textureCompression").as_int());
}

ermy::rendering::Format TextureAsset::FormatFromTextureCompression(TextureCompression compression)
{
	using namespace ermy::rendering;
	
	switch(compression)
	{
		case TextureCompression::TC_BC1_SRGB:
			return ermy::rendering::Format::BC1;
		case TextureCompression::TC_BC1_UNORM:
			return ermy::rendering::Format::BC1;
		case TextureCompression::TC_BC5_UNORM:
			return ermy::rendering::Format::BC5;
		case TextureCompression::TC_BC6H_UNORM:
			return ermy::rendering::Format::BC6;
		default:
			return ermy::rendering::Format::UNKNOWN;
	}
}
	

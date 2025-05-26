#include <assets/font_asset.h>
#include <imgui.h>
#include <preview_renderer.h>
#include <editor_shader_internal.h>
#include <editor_file_utils.h>

using namespace ermy;
static char previewBuff[256] = {};

struct FontLivePreviewParams
{
	int numChannels;
	float position;
	float volume;
};

class FontRenderPreview
{
	FontRenderPreview();
	~FontRenderPreview();

	rendering::PSOID fullscreenWavefront;
public:
	static FontRenderPreview& Instance()
	{
		static FontRenderPreview instance;
		return instance;
	}

	void BindPSO(rendering::CommandList& cl, bool isStatic = false)
	{
		cl.SetPSO(fullscreenWavefront);
	}
};

FontRenderPreview::FontRenderPreview()
{
	auto RTT = PreviewRenderer::Instance().GetRTT();
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	//live preview PSO
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSSoundWavefront());
		desc.specificRenderPass = RTT;
		desc.uniforms.push_back(rendering::ShaderUniformType::Texture2D);
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(FontRenderPreview));
		desc.debugName = "FullscreenWavefront";
		fullscreenWavefront = rendering::CreatePSO(desc);
	}
}

FontRenderPreview::~FontRenderPreview()
{

}

FontAsset::FontAsset()
{

}

FontAsset::~FontAsset()
{
	if(font)
	{
		delete font;
		font = nullptr;
	}
}

void FontAsset::DrawPreview()
{
	if(!font)
		return;

	ImGui::Text("Font: %lf", font->GetNumberOfGlyphs());

	//font->GetType()
	//if (imFont)
	//{
	//	ImGui::PushFont(imFont);
	//	ImGui::InputTextMultiline("Preview", previewBuff, sizeof(previewBuff));
	//	ImGui::PopFont();
	//}	
}

void FontAsset::LoadFromCachedRaw(std::ifstream& file, const std::filesystem::path& path)
{
	_createLivePreviewTexture();
}

void FontAsset::SaveToCachedRaw(std::ofstream& file)
{
	//writeBinary(file, duration);
	//writeBinary(file, channels);
	//writeVector(file, samplesPreviewRaw);
}

void FontAsset::_createLivePreviewTexture()
{	
}

void FontAsset::RegenerateLivePreview()
{	
	_createLivePreviewTexture();
}

void FontAsset::RenderStaticPreview(ermy::rendering::CommandList& cl)
{
	
}

std::vector<ermy::u8> FontAsset::GetStaticPreviewData()
{
	if (!font)
		return std::vector<ermy::u8>();

	auto previewRGBAData = font->GeneratePreviewRGBA(128, 128);

	return PreviewRenderer::Instance().ConvertRGBADataToBC1(previewRGBAData);

	return PreviewRenderer::Instance().GetPreviewDataBC1([&](ermy::rendering::CommandList& cl)
	{
		RenderStaticPreview(cl);
	});
}

void FontAsset::RenderPreview(ermy::rendering::CommandList& cl)
{

}

void FontAsset::MouseZoom(float dv)
{

}
void FontAsset::MouseDown(float normalizedX, float normalizedY, int button)
{

}

void FontAsset::Save(pugi::xml_node& node)
{
	AssetData::Save(node);
}

void FontAsset::Load(pugi::xml_node& node)
{
	AssetData::Load(node);
}

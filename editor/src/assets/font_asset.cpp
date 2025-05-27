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
    if (!font)
        return;

    ImGui::Text("Number of glyphs: %d", font->GetNumberOfGlyphs());

    const char* atlasTypeNames[3] = { "Grayscale", "SDF", "MTSDF" };

    if (ImGui::Button("Add Atlas"))
    {
        atlases.push_back(FontAtlas{});
    }

    if (ImGui::Button("Add Vertex Cache"))
    {
        vertexCaches.push_back(FontVertexCache{});
    }
    ImGui::Separator();

    static ImVec2 magnifiedRegionUVMin(0.0f, 0.0f);
    static ImVec2 magnifiedRegionUVMax(1.0f, 1.0f);
    static bool isMagnified = false;

    int curIndex = 0;
    for (auto& atlas : atlases)
    {
        ImGui::PushID(curIndex);

        ImGui::InputText("Atlas Name", atlas.name, 64);

        int curType = (int)atlas.type;
        ImGui::Combo("Atlas Type", &curType, atlasTypeNames, 3, 3);
        atlas.type = (ermy::Font::AtlasType)curType;

        ImGui::InputInt("Atlas Size", &atlas.glyphSize, 8, 128);

        if (atlas.aresult)
        {
            if (atlas.aresult->isDone)
            {
                if (!atlas.previewAtlasTex.isValid())
                {
                    auto texInfo = atlas.aresult->result;

                    auto desc = ermy::rendering::TextureDesc::Create2D(texInfo.width, texInfo.height, texInfo.format);
                    desc.dataSize = texInfo.pixelsData.size();
                    desc.pixelsData = texInfo.pixelsData.data();
                    desc.debugName = "Font Atlas";

                    atlas.previewAtlasTex = ermy::rendering::CreateDedicatedTexture(desc);
                    atlas.previewAtlasTexImgui = ermy::rendering::GetTextureDescriptor(atlas.previewAtlasTex);
                }

                if (atlas.previewAtlasTex.isValid())
                {
                    // Define the size of the image
                    ImVec2 imageSize(512, 512);

                    // Create an invisible button over the image to make it interactable
                    auto cpos = ImGui::GetCursorPos();
                    ImGui::InvisibleButton("ImageButton", imageSize);

                    // Check if the mouse is active on the invisible button
                    if (ImGui::IsItemActive())
                    {
                        ImVec2 mousePos = ImGui::GetMousePos();
                        ImVec2 imagePos = ImGui::GetItemRectMin();
                        ImVec2 imageSize = ImGui::GetItemRectSize();

                        // Calculate UV coordinates of the magnified region
                        ImVec2 uv = ImVec2(
                            (mousePos.x - imagePos.x) / imageSize.x,
                            (mousePos.y - imagePos.y) / imageSize.y
                        );

                        // Define the magnified region (e.g., 0.1x0.1 around the clicked point)
                        float magnificationSize = 0.1f;
                        magnifiedRegionUVMin = ImVec2(
                            std::max(0.0f, uv.x - magnificationSize / 2.0f),
                            std::max(0.0f, uv.y - magnificationSize / 2.0f)
                        );
                        magnifiedRegionUVMax = ImVec2(
                            std::min(1.0f, uv.x + magnificationSize / 2.0f),
                            std::min(1.0f, uv.y + magnificationSize / 2.0f)
                        );

                        if (magnifiedRegionUVMax.x < magnificationSize)
                            magnifiedRegionUVMax.x = magnificationSize;

                        if(magnifiedRegionUVMax.y < magnificationSize)
                            magnifiedRegionUVMax.y = magnificationSize;

                        if (magnifiedRegionUVMin.x > 1.0f - magnificationSize)
                            magnifiedRegionUVMin.x = 1.0f - magnificationSize;

                        if (magnifiedRegionUVMin.y > 1.0f - magnificationSize)
                            magnifiedRegionUVMin.y = 1.0f - magnificationSize;

                        isMagnified = true;
                    }
                    else
                    {
                        isMagnified = false;
                    }
                    ImGui::SetCursorPos(cpos);
                    // Draw the magnified region or the whole image
                    if (isMagnified)
                    {
                        ImGui::Image(atlas.previewAtlasTexImgui, imageSize, magnifiedRegionUVMin, magnifiedRegionUVMax);
                    }
                    else
                    {
                        ImGui::Image(atlas.previewAtlasTexImgui, imageSize);
                    }
                }
            }
            else
            {
                ImGui::ProgressBar(atlas.aresult->progress);
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    // Handle cancel
                }
            }
        }
        else
        {
            if (ImGui::Button("Generate Atlas"))
            {
                atlas.atlas = font->CreateAtlas(atlas.type, atlas.glyphSize);
                atlas.aresult = new ermy::Font::ASyncAtlasResult();
                atlas.atlas->GenerateFullAtlasASync(atlas.aresult);
            }
        }

        ImGui::Separator();

        ImGui::PopID();
        curIndex++;
    }

    // Draw all vertex caches
    curIndex = 0;
    for (auto& vertexCache : vertexCaches)
    {
        ImGui::PushID(curIndex); // Push unique ID for this vertex cache

        ImGui::InputText("Vertex Cache Name", vertexCache.name, 64);

        if (ImGui::Button("Generate Vertex Cache"))
        {
            // Handle vertex cache generation
        }

        ImGui::Separator();

        ImGui::PopID(); // Pop unique ID
        curIndex++;
    }
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

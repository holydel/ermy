#include <assets/sound_asset.h>
#include <imgui.h>
#include <preview_renderer.h>
#include <editor_shader_internal.h>
using namespace ermy;

struct SoundLivePreviewParams
{
	int numChannels;
	float position;
	float volume;
};

class SoundRenderPreview
{
	SoundRenderPreview();
	~SoundRenderPreview();

	rendering::PSOID fullscreenWavefront;	
public:
	static SoundRenderPreview& Instance()
	{
		static SoundRenderPreview instance;
		return instance;
	}

	void BindPSO(rendering::CommandList& cl, bool isStatic = false)
	{
		cl.SetPSO(fullscreenWavefront);
	}
};

SoundRenderPreview::SoundRenderPreview()
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
		desc.AddRootConstantRange(ShaderStage::Fragment, sizeof(SoundLivePreviewParams));
		desc.debugName = "FullscreenWavefront";
		fullscreenWavefront = rendering::CreatePSO(desc);
	}
}

SoundRenderPreview::~SoundRenderPreview()
{

}

SoundAsset::SoundAsset()
{

}

SoundAsset::~SoundAsset()
{

}

void SoundAsset::DrawPreview()
{
	ImGui::Text("Sound duration: %lf", duration);
	
	if (ermy::sound::IsPlaying(sound))
	{		
		if (ImGui::Button("Stop"))
		{
			ermy::sound::Stop(sound);
		}

		ImGui::SameLine();
		//float currentTime = ermy::sound::GetCurrentPlaybackTime(sound);
		//ImGui::ProgressBar(currentTime / duration);
	}
	else
	{
		if (ImGui::Button("Play"))
		{
			ermy::sound::Play(sound);
		}
	}
}

void SoundAsset::RegenerateLivePreview()
{
	u8* data = new u8[512 * channels];
	float* maxGain = new float[channels];
	float* avgGain = new float[channels];

	int samplesInBar = samples.size() / 512 / channels;

	for (int s = 0; s < 512; s++)
	{
		memset(maxGain, 0, sizeof(float) * channels);
		memset(avgGain, 0, sizeof(float) * channels);

		for (int i = 0; i < samplesInBar; i++)
		{
			for (int c = 0; c < channels; c++)
			{
				float v = abs(samples[(s * samplesInBar + i) * channels + c]);
				avgGain[c] += v;
				if (v > maxGain[c])
					maxGain[c] = v;
			}
		}

		for (int c = 0; c < channels; c++)
		{
			avgGain[c] /= samplesInBar;

			data[s + c * 512] = std::min((int)((avgGain[c]) * 512.0f),255);
			int a = 2;
		}
	}

	ermy::rendering::TextureDesc descLive;
	descLive.width = 512;
	descLive.height = channels;
	descLive.depth = 1;
	descLive.isCubemap = false;
	descLive.numLayers = 1;
	descLive.numMips = 1;
	descLive.pixelsData = data;
	descLive.isSparse = false;
	descLive.texelSourceFormat = ermy::rendering::Format::R8_UNORM;
	descLive.dataSize = 512 * channels;

	previewTextureLive = ermy::rendering::CreateDedicatedTexture(descLive);

	ermy::rendering::TextureDesc descStatic;
	descStatic.width = 128;
	descStatic.height = 128;
	descStatic.depth = 1;
	descStatic.isCubemap = false;
	descStatic.numLayers = 1;
	descStatic.numMips = 1;
	descStatic.isSparse = false;
	descStatic.texelSourceFormat = ermy::rendering::Format::RGBA8_UNORM;

	descStatic.pixelsData = nullptr;
	descStatic.dataSize = 0;

	//previewTextureStatic = ermy::rendering::CreateDedicatedTexture(descStatic);
	assetPreviewTexLive = ermy::rendering::GetTextureDescriptor(previewTextureLive);
	//assetPreviewTexStatic = ermy::rendering::GetTextureDescriptor(previewTextureStatic);

	PreviewRenderer::Instance().EnqueueStaticPreviewGeneration(this);
}

void SoundAsset::RenderStaticPreview(ermy::rendering::CommandList& cl)
{
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	cl.BeginRenderPass(staticRTT);
	SoundRenderPreview::Instance().BindPSO(cl);
	cl.SetDescriptorSet(0, assetPreviewTexLive);
	SoundLivePreviewParams pass;
	pass.numChannels = channels;
	pass.position = 0;
	pass.volume = 1.0f;
	cl.SetRootConstant(pass, ShaderStage::Fragment);
	cl.Draw(3);
	cl.EndRenderPass();

	auto staticRTTTex = PreviewRenderer::Instance().GetStaticTexture();
	//cl.BlitTexture(staticRTTTex, previewTextureStatic);
	int a = 42;
}

void SoundAsset::RenderPreview(ermy::rendering::CommandList& cl)
{
	SoundRenderPreview::Instance().BindPSO(cl);
	cl.SetDescriptorSet(0, assetPreviewTexLive);
	SoundLivePreviewParams pass;
	pass.numChannels = channels;
	pass.position = (ermy::sound::GetCurrentPlaybackTime(sound) / duration);
	pass.volume = volume;
	cl.SetRootConstant(pass, ShaderStage::Fragment);
	cl.Draw(3);
}

void SoundAsset::MouseZoom(float dv)
{
	volume *= dv;
	ermy::sound::SetVolume(sound, volume);
}
void SoundAsset::MouseDown(float normalizedX, float normalizedY, int button)
{
	if (button == 0)
	{
		ermy::sound::SetPlayingOffset(sound, normalizedX * duration);
	}	
}

void SoundAsset::Save(pugi::xml_node& node)
{
	AssetData::Save(node);
}

void SoundAsset::Load(pugi::xml_node& node)
{
	AssetData::Load(node);
}

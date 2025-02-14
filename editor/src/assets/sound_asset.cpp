#include <assets/sound_asset.h>
#include <imgui.h>

SoundAsset::SoundAsset()
{

}

SoundAsset::~SoundAsset()
{

}

void SoundAsset::DrawPreview()
{
	ImGui::Text("Sound duration: %lf", duration);
	ImGui::Button("Play");
}

//std::vector<std::string> SoundAsset::Initialize()
//{
//	return std::vector<std::string>({ "wav","mp3","flac" });
//}
//
//void SoundAsset::Shutdown()
//{
//
//}
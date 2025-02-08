#include <assets/sound_asset.h>

SoundAsset::SoundAsset(const char* filepath)
{

}

SoundAsset::~SoundAsset()
{

}

std::vector<std::string> SoundAsset::Initialize()
{
	return std::vector<std::string>({ "wav","mp3","flac" });
}

void SoundAsset::Shutdown()
{

}
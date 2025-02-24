#pragma once
#include "assets/SoundsLoader.h"

#include "assets/sound_asset.h"
#define MA_ENABLE_FLAC 
#define MA_ENABLE_MP3
#include "../../third_party/miniaudio/miniaudio.h"

using namespace editor::asset::loader;
using namespace ermy;

static std::vector<std::string> gSupportedExtensions;

const std::vector<std::string>& SoundsLoader::SupportedExtensions()
{
	gSupportedExtensions.push_back("wav");
	gSupportedExtensions.push_back("mp3");
	gSupportedExtensions.push_back("flac");
	return gSupportedExtensions;
}


SoundsLoader::SoundsLoader()
{

}

SoundsLoader::~SoundsLoader()
{

}

int get_format_sample_size(ma_format format) {
	switch (format) {
	case ma_format_u8: {
		return 1;;
	}
	case ma_format_s16: {
		return 2;
	}
	case ma_format_s24: {
		return 3;
	}
	}
	return 4;

}

void convert_to_float(const void* pFrames, ma_format format, ma_uint64 frameCount, ma_uint32 channels, std::vector<float>& output) {
	switch (format) {
	case ma_format_u8: {
		const ma_uint8* pData = (const ma_uint8*)pFrames;
		for (ma_uint64 i = 0; i < frameCount * channels; ++i) {
			output.push_back((pData[i] - 128) / 128.0f); // Convert u8 to float
		}
		break;
	}
	case ma_format_s16: {
		const ma_int16* pData = (const ma_int16*)pFrames;
		for (ma_uint64 i = 0; i < frameCount * channels; ++i) {
			output.push_back(pData[i] / 32768.0f); // Convert s16 to float
		}
		break;
	}
	case ma_format_s24: {
		const ma_uint8* pData = (const ma_uint8*)pFrames;
		for (ma_uint64 i = 0; i < frameCount * channels; ++i) {
			ma_int32 sample = (pData[i * 3 + 0] << 8) | (pData[i * 3 + 1] << 16) | (pData[i * 3 + 2] << 24);
			output.push_back((float)(sample >> 8) / 8388608.0f); // Convert s24 to float
		}
		break;
	}
	case ma_format_s32: {
		const ma_int32* pData = (const ma_int32*)pFrames;
		for (ma_uint64 i = 0; i < frameCount * channels; ++i) {
			output.push_back(pData[i] / 2147483648.0f); // Convert s32 to float
		}
		break;
	}
	case ma_format_f32: {
		const float* pData = (const float*)pFrames;
		output.insert(output.end(), pData, pData + frameCount * channels); // No conversion needed
		break;
	}
	default:
		break;
	}
}

AssetData* SoundsLoader::Load(const std::filesystem::path& path)
{
	SoundAsset* result = new SoundAsset();

	result->sound = ermy::sound::LoadFromFile(path.string().c_str());
	result->duration = ermy::sound::GetDuration(result->sound);

	ma_result result2;
	ma_decoder decoder;

	// Replace "your_audio_file.wav" with the path to your audio file
	result2 = ma_decoder_init_file(path.string().c_str(), NULL, &decoder);

	// Get the number of channels and sample rate
	ma_uint32 channels = decoder.outputChannels;
	ma_uint32 sampleRate = decoder.outputSampleRate;

	// Calculate the total number of samples
	ma_uint64 totalSamples;
	ma_decoder_get_length_in_pcm_frames(&decoder, &totalSamples);

	// Create a buffer to hold all samples for all channels
	result->samples.clear();
	result->samples.reserve(totalSamples * channels);
	// Read all samples into the buffer
	ma_uint64 framesRead = 0;
	int totalSize = get_format_sample_size(decoder.outputFormat) * totalSamples * channels;
	void* tempBuff = ma_aligned_malloc(totalSize,16,nullptr);
	auto res3 = ma_decoder_read_pcm_frames(&decoder, tempBuff, totalSamples, &framesRead);
	convert_to_float(tempBuff, decoder.outputFormat, framesRead, channels, result->samples);

	ma_aligned_free(tempBuff, nullptr);
	// Clean up
	ma_decoder_uninit(&decoder);
	result->channels = channels;
	result->RegeneratePreview();

	return result;
}
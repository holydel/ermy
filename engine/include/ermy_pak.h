#pragma once
#include <ermy_api.h>
#include <ermy_rendering.h>

namespace ermy
{
	namespace pak
	{
		struct TextureRawInfo
		{
			u32 dataSize = 0;
			u16 width = 1;
			u16 height = 1;
			u16 depth = 1;
			u16 numLayers = 1;
			u8 numMips = 1;
			rendering::Format texelSourceFormat = rendering::Format::RGBA8_UNORM;

			//flags
			bool isCubemap : 1 = false;
		};

		struct SoundRawInfo
		{
			u32 dataSize = 0;			
			u32 sampleRate = 44100;
			u8 numChannels = 1;

			//flags
			bool isStreaming : 1 = false;
		};
		constexpr u32 PAK_MAGIC = 0x4B415045; // "EPAK"

		bool MountPak(const char* filePathUtf8);
	}
}
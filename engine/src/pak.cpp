#include "pak.h"
#include <cstdio>
#include <fstream>
#include <ermy_os_utils.h>
#include <ermy_mapped_reader.h>
#include <ermy_sound.h>

using namespace ermy;
using namespace ermy::pak;

ermy::os_utils::MappedFileHandle mappedPak;
void* mappedData = nullptr;

bool ermy::pak::MountPak(const char* filePathUtf8)
{
	mappedPak = ermy::os_utils::MapFileReadOnly(filePathUtf8);
	if (!mappedPak)
		return true;

	mappedData = ermy::os_utils::GetPointer(mappedPak);

	MappedFileReader reader(mappedData);

	u32 pakMagic = reader.ReadU32();

	if (pakMagic != PAK_MAGIC)
	{
		return false;
	}

	std::vector<pak::TextureRawInfo> texMeta;
	std::vector<pak::SoundRawInfo> soundMeta;

	reader.ReadVector(texMeta);
	reader.ReadVector(soundMeta);

	for (int i = 0; i < texMeta.size(); ++i)
	{
		const pak::TextureRawInfo& raw = texMeta[i];
		rendering::TextureDesc tdesc;
		tdesc.dataSize = raw.dataSize;
		tdesc.depth = raw.depth;
		tdesc.height = raw.height;
		tdesc.isCubemap = raw.isCubemap;
		tdesc.numLayers = raw.numLayers;
		tdesc.numMips = raw.numMips;
		tdesc.texelSourceFormat = raw.texelSourceFormat;
		tdesc.width = raw.width;
		tdesc.pixelsData = reader.CurrentPointer();

		auto tid = rendering::CreateDedicatedTexture(tdesc);

		reader.Advance(raw.dataSize);
	}

	for (int i = 0; i < soundMeta.size(); ++i)
	{
		const pak::SoundRawInfo& raw = soundMeta[i];

		auto sid = sound::LoadFromMemory(reader.CurrentPointer(), raw.dataSize);
		reader.Advance(raw.dataSize);

		sound::Play(sid);
		sound::SetVolume(sid, 0.02f);
	}

	return true;
}
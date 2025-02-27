#include "sound_interface.h"

#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_FLAC 
#define MA_ENABLE_MP3
#include "../../third_party/miniaudio/miniaudio.h"
#include <ermy_log.h>
#include <vector>
static ma_engine gMAEngine;
static ma_context gMACtx;
using namespace ermy;
using namespace ermy::sound;

std::vector<ma_sound*> gSounds;

void sound_interface::Initialize()
{	
	ma_context_config ctxConfig = ma_context_config_init();
	ma_result result = ma_context_init(NULL, 0, &ctxConfig, &gMACtx);

	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_context_get_devices(&gMACtx, &pPlaybackDeviceInfos, &playbackDeviceCount, nullptr, nullptr);

	for (int i = 0; i < playbackDeviceCount; ++i)
	{
		const auto& device = pPlaybackDeviceInfos[i];
		ERMY_LOG("Device %d: %s %s", i, device.name, device.isDefault ? " (DEFAULT)" : "");
	}

	ma_engine_config engineConfig = ma_engine_config_init();
	result = ma_engine_init(&engineConfig, &gMAEngine);

	//const char* f1 = "D:\\Projects\\ermy\\eproj_template\\assets\\sounds\\file_example_MP3_2MG.mp3";
	//const char* f2 = "D:\\Projects\\ermy\\eproj_template\\assets\\sounds\\file_example_WAV_1MG.wav";
	//const char* f3 = "D:\\Projects\\ermy\\eproj_template\\assets\\sounds\\sample3.flac";

	//result = ma_engine_play_sound(&gMAEngine, f1, nullptr);
	//result = ma_engine_play_sound(&gMAEngine, f2, nullptr);
	//result = ma_engine_play_sound(&gMAEngine, f3, nullptr);
}

void sound_interface::Shutdown()
{
	ma_engine_uninit(&gMAEngine);
}

void sound_interface::Process()
{
}

void sound_interface::BeginFrame()
{
}

void sound_interface::EndFrame()
{
}

SoundID sound::LoadFromFile(const char* filename)
{
	SoundID result = { static_cast<u32>(gSounds.size()) };

	ma_sound* pSound = new ma_sound();
	ma_result initResult = ma_sound_init_from_file(&gMAEngine, filename, 0, nullptr, nullptr, pSound);

	if (initResult != MA_SUCCESS) {
		delete pSound;
		return { SoundID::InvalidValue };
	}

	gSounds.push_back(pSound);
	return result;
}

SoundID sound::LoadFromMemory(const void* dataPtr, u32 filesize)
{
	SoundID result = { static_cast<u32>(gSounds.size()) };

	ma_sound* pSound = new ma_sound();

	ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_unknown, 0, 0);
	ma_decoder* decoder = new ma_decoder();
	ma_result initResult = ma_decoder_init_memory(dataPtr, filesize, &decoderConfig, decoder);
	if (initResult != MA_SUCCESS) {
		delete pSound;
		return { SoundID::InvalidValue };
	}

	ma_sound_init_from_data_source(&gMAEngine, decoder, 0, nullptr, pSound);

	gSounds.push_back(pSound);

	return result;
}

void sound::Play(SoundID sound)
{
	ma_sound_start(gSounds[sound.handle]);
}

void sound::Stop(SoundID sound)
{
	ma_sound_stop(gSounds[sound.handle]);
}

void sound::SetVolume(SoundID sound, float volume)
{
	ma_sound_set_volume(gSounds[sound.handle], volume);
}
void sound::SetPitch(SoundID sound, float pitch)
{
	ma_sound_set_pitch(gSounds[sound.handle], pitch);
}
void sound::SetLooping(SoundID sound, bool looping)
{
	ma_sound_set_looping(gSounds[sound.handle], looping);
}
void sound::SetPosition(SoundID sound, float x, float y, float z)
{
	ma_sound_set_position(gSounds[sound.handle], x, y, z);
}
void sound::SetVelocity(SoundID sound, float x, float y, float z)
{
	ma_sound_set_velocity(gSounds[sound.handle], x, y, z);
}
void sound::SetDirection(SoundID sound, float x, float y, float z)
{
	ma_sound_set_direction(gSounds[sound.handle], x, y, z);
}
void sound::SetCone(SoundID sound, float innerAngle, float outerAngle, float volume)
{
	ma_sound_set_cone(gSounds[sound.handle], innerAngle, outerAngle, volume);
}

float  sound::GetDuration(SoundID sound)
{
	float length = 0.0f;
	ma_sound_get_length_in_seconds(gSounds[sound.handle],&length);
	return length;
}

bool sound::IsPlaying(SoundID sound)
{
	return ma_sound_is_playing(gSounds[sound.handle]);
}

float sound::GetCurrentPlaybackTime(SoundID sound)
{
	double ms = static_cast<double>(ma_sound_get_time_in_milliseconds(gSounds[sound.handle]));
	
	return static_cast<float>(ms / 1000.0);
}

void sound::SetPlayingOffset(SoundID sound, float offset)
{
	if (offset < 0)
		offset = 0;

	u64 numPCMFrames = 0;
	ma_sound_get_length_in_pcm_frames(gSounds[sound.handle],&numPCMFrames);
	if (numPCMFrames == 0)
		return;

	//ma_sound_get_data_format(gSounds[sound.handle], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	u64 pcmFrame = static_cast<u64>(offset * numPCMFrames / GetDuration(sound));

	if (pcmFrame >= numPCMFrames)
		pcmFrame = numPCMFrames - 1;
	
	ma_sound_seek_to_pcm_frame(gSounds[sound.handle], pcmFrame);
}
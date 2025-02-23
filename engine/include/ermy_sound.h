#pragma once

#include <ermy_api.h>

namespace ermy
{
	namespace sound
	{
		typedef Handle32 SoundID;

		SoundID LoadFromFile(const char* filename);
		void Play(SoundID sound);
		void Stop(SoundID sound);
		void SetVolume(SoundID sound, float volume);
		void SetPitch(SoundID sound, float pitch);
		void SetLooping(SoundID sound, bool looping);
		void SetPosition(SoundID sound, float x, float y, float z);
		void SetVelocity(SoundID sound, float x, float y, float z);
		void SetDirection(SoundID sound, float x, float y, float z);
		void SetCone(SoundID sound, float innerAngle, float outerAngle, float volume);		
	}
}
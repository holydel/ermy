#pragma once
#include <ermy_api.h>
#include <ermy_sound.h>

namespace sound_interface
{
	void Initialize();
	void Shutdown();
	void Process();
	void BeginFrame();
	void EndFrame();
}
#pragma once

#include <ermy_api.h>

namespace imgui_interface
{
	void Initialize();
	void Shutdown();
	void BeginFrame(void* cmdList);
	void EndFrame(void* cmdList);
	void Render();
}
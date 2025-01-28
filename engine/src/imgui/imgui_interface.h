#pragma once

#include <ermy_api.h>

namespace imgui_interface
{
	void Initialize();
	void Shutdown();
	void NewFrame(void* cmdList);
	void Render();
}
#pragma once

#include "ermy_api.h"


namespace rendering
{
	void BeginFrame();
	void Initialize();
	void Shutdown();
	void Process();
	void EndFrame();
}
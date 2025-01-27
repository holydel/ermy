#pragma once
#include <ermy_api.h>

namespace framegraph_interface
{
	void Initialize(ermy::u8 numFrames);
	void Shutdown();
	void Process();
};
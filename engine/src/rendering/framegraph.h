#pragma once

#include "framegraph_interface.h"
#include <ermy_rendering.h>

namespace framegraph
{
	void Initialize(ermy::u8 numFrames);
	void Shutdown();
	void Process();
};

extern ermy::rendering::BufferID gFrameConstants;
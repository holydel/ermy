#pragma once

#include "framegraph_interface.h"

namespace framegraph
{
	void Initialize(ermy::u8 numFrames);
	void Shutdown();
	void Process();
};
#pragma once
#include "ermy_api.h"

namespace ermy
{
	enum class ShaderStage : u8
	{
		Vertex,
		Fragment,
		Compute,
	};

	struct ShaderBytecode
	{
		u8* data;
		u32 size;
		ShaderStage stage;
		bool isInternal;
		mutable Handle16 cachedDeviceObjectID;
	};
}
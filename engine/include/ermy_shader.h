#pragma once
#include "ermy_api.h"
#include <string>

namespace ermy
{
	enum class ShaderDomainTag : u8
	{
		Internal,
		Editor,
		Runtime,
		Application
	};

	enum class ShaderStage : u8
	{
		Vertex,
		Fragment,
		Compute,

		MAX
	};

	struct ShaderBytecode
	{
		u8* data;
		u32 size;
		ShaderStage stage;
		bool isInternal;
		mutable Handle16 cachedDeviceObjectID;
	};

	struct ShaderInfo
	{
		ShaderBytecode byteCode;
		std::string shaderName;
		ShaderDomainTag tag;
		u64 bytecodeCRC64;
	};
}
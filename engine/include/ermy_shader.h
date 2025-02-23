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
		Application,

		UNKNOWN
	};

	enum class ShaderStage : u8
	{
		Vertex,
		Fragment,
		Compute,
		Geometry,

		MAX
	};

	struct ShaderBytecode
	{
		const u8* data = nullptr;
		u32 size = 0;
		ShaderStage stage = ShaderStage::MAX;
		bool isInternal  = true;
		mutable Handle16 cachedDeviceObjectID;
	};

	struct ShaderInfo
	{
		ShaderBytecode byteCode;
		std::string shaderName  ="";
		ShaderDomainTag tag = ShaderDomainTag::UNKNOWN;
		u64 bytecodeCRC64 = 0;;
	};
}
#pragma once
#include <ermy_api.h>
#include <vector>
#include <ermy_shader.h>

namespace ermy
{
	namespace rendering
	{
		typedef Handle32 PSOID;

		struct PSODesc
		{
			std::vector<ShaderBytecode> shaders;
		};

		PSOID CreatePSO(const PSODesc &desc);
	}
}
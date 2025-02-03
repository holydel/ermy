#pragma once
#include <ermy_api.h>
#include <vector>
#include <ermy_shader.h>

namespace ermy
{
	namespace rendering
	{
		enum class PrimitiveTopology
		{
			TriangleList,
			TriangleStrip,
			Points,
			LineList,
			LineStrip
		};

		typedef Handle32 PSOID;

		struct PSODesc
		{
			std::vector<ShaderBytecode> shaders;
			PrimitiveTopology topology = PrimitiveTopology::TriangleList;
		};

		PSOID CreatePSO(const PSODesc &desc);
	}
}
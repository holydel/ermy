#pragma once
#include "ermy_shader.h"

namespace ermy
{
	namespace shader_internal
	{
		ShaderBytecode computeMain();
		ShaderBytecode testTriangleVS();
		ShaderBytecode testTriangleFS();
	}
}
#pragma once
#include "ermy_shader.h"

namespace ermy
{
	namespace shader_internal
	{
		ShaderInfo computeMain();
		ShaderInfo dedicatedSpriteVS();
		ShaderInfo testTriangleVS();
		ShaderInfo testTriangleFS();
		ShaderInfo dedicatedSpriteFS();
	}
}
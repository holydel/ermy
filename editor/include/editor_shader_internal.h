#pragma once
#include "ermy_shader.h"

namespace ermy
{
	namespace shader_internal
	{
		ShaderBytecode dedicatedStaticMeshVS();
		ShaderBytecode dedicatedStaticMeshFS_UV0();
		ShaderBytecode fullscreenVS();
		ShaderBytecode fullscreenFSEmpty();
		ShaderBytecode fullscreenFS2D();
		ShaderBytecode fullscreenFSCubemap();
		ShaderBytecode fullscreenFS2DStatic();
		ShaderBytecode fullscreenFSCubemapStatic();
	}
}
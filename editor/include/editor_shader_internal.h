#pragma once
#include "ermy_shader.h"

namespace ermy
{
	namespace shader_editor
	{
		ShaderInfo dedicatedStaticMeshVS();
		ShaderInfo dedicatedStaticMeshFS();
		ShaderInfo fullscreenVS();
		ShaderInfo fullscreenFSEmpty();
		ShaderInfo fullscreenFS2D();
		ShaderInfo fullscreenFSCubemap();
		ShaderInfo fullscreenFS2DArray();
		ShaderInfo fullscreenFSCubemapArray();
		ShaderInfo fullscreenFS2DStatic();
		ShaderInfo fullscreenFS2DArrayStatic();
		ShaderInfo fullscreenFSCubemapStatic();
		ShaderInfo fullscreenFSCubemapArrayStatic();
		ShaderInfo fullscreenFSVolumetric();
		ShaderInfo fullscreenFSVolumetricStatic();
	}
}
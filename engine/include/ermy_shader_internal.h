#pragma once
#include "ermy_shader.h"

namespace ermy
{
	namespace shader_internal
	{
		ShaderInfo dedicatedSpriteVS();
		ShaderInfo testTriangleVS();
		ShaderInfo testTriangleFS();
		ShaderInfo dedicatedSpriteFS();
		ShaderInfo fullscreenVS();
		ShaderInfo previewStereoSwapchain();
		ShaderInfo skyboxVS();
		ShaderInfo skyboxStereoVS();
		ShaderInfo skyboxFS();
		ShaderInfo sceneStaticMeshVS();
		ShaderInfo sceneStaticMeshStereoVS();
		ShaderInfo sceneStaticMeshColored();
	}
}
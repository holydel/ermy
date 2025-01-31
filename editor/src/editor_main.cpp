#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
#include "main_window.h"
#include "project/eproject.h"
#include "shader_compiler/shader_compiler.h"

class Editor : public ermy::Application
{
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "Ermy Editor";

		staticConfig.imgui.enable = true;
		staticConfig.imgui.enableDocking = true;
	}

	void OnLoad() override
	{
		ShaderCompiler::Instance().Initialize();
		ErmyProject::Instance().MountToLocalDirectory("../../eproj_template");

		ErmyProject::Instance().RecompileAllShaders();
	}
	void OnIMGUI() override
	{
		main_window::Draw();
	}
};

static Editor editor;
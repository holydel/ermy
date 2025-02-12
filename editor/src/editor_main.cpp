#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
#include "main_window.h"
#include "project/eproject.h"
#include "shader_compiler/shader_compiler.h"
#include <assets/assets_importer.h>

bool gIsEditorRunning = true;

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

		ErmyProject::Instance().RecompileAllInternalShaders();
		assets_importer::Initialize();

		//assets_importer::Import("C:\\Users\\holyd\\Pictures\\m92pjmfggseb1.webp", AssetType::Texture);
		//assets_importer::Import("C:\\Users\\holyd\\Downloads\\glTF-Sample-Models-master\\sourceModels\\BarramundiFish\\BarramundiFish.fbx", AssetType::Geometry);
	}

	void OnIMGUI() override
	{
		main_window::Draw();
	}

	void OnApplicationClose() override
	{

	}

	bool IsRunning() override
	{
		return gIsEditorRunning;
	}
};

static Editor editor;
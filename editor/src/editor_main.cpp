#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
#include "main_window.h"
#include "project/eproject.h"
#include "shader_compiler/shader_compiler.h"
#include <assets/assets_loader.h>
#include "preview_renderer.h"

bool gIsEditorRunning = true;

class Editor : public ermy::Application
{

public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "Ermy Editor";
		//staticConfig.window.initialState = ermy::Application::StaticConfig::WindowConfig::InitialState::Maximized;
		staticConfig.render.adapterID = 0;
		staticConfig.render.enableDebugLayers = true;

		staticConfig.imgui.enable = true;
		staticConfig.imgui.enableDocking = true;

		staticConfig.render.enableBarycentricFS = true;
		staticConfig.render.enableGeometryShader = true;
		staticConfig.render.enableMeshShader = true;
		staticConfig.render.enableRaytracing = true;
		staticConfig.render.enableSamplerYCbCr = true;
	}

	void OnLoad() override
	{
		ShaderCompiler::Instance().Initialize();
		ErmyProject::Instance().MountToLocalDirectory("../../eproj_template");

		editor::asset::loader::AssetsLoader::Initialize();

		editor::screen::AssetsBrowserScreen::Instance()->Show();
		editor::screen::HierarchyScreen::Instance()->Show();
		editor::screen::PropertyEditorScreen::Instance()->Show();
		//assets_importer::Import("C:\\Users\\holyd\\Pictures\\m92pjmfggseb1.webp", AssetType::Texture);
		//assets_importer::Import("C:\\Users\\holyd\\Downloads\\glTF-Sample-Models-master\\sourceModels\\BarramundiFish\\BarramundiFish.fbx", AssetType::Geometry);
	}

	void OnBeginFrame(ermy::rendering::CommandList& finalCL) override
	{
		PreviewRenderer::Instance().RenderPreview(finalCL);
	}

	void OnIMGUI() override
	{
		main_window::Draw();
	}

	void OnApplicationClose() override
	{
		editor::asset::loader::AssetsLoader::Shutdown();
	}

	bool IsRunning() override
	{
		return gIsEditorRunning;
	}
};

static Editor gEditor;
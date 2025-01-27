#include "ermy_imgui.h"
#include "imgui_interface.h"
#include "../os/os.h"
#include "application.h"

void imgui::Initialize()
{
	auto& app = GetApplication();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	if (app.staticConfig.imgui.enableDocking)
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch	

}

void imgui::Shutdown()
{

}

void imgui::NewFrame()
{

}

void imgui::Render()
{

}
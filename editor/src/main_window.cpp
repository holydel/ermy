#include "main_window.h"
#include <imgui.h>
#include <imgui_internal.h>

#include "project/eproject.h"
#include "shader_compiler/shader_compiler.h"

extern bool gIsEditorRunning;

namespace main_window
{
	void DrawMainMenu()
	{
		//Imgui draw main menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New"))
				{
					//New file
				}
				if (ImGui::MenuItem("Open"))
				{
					//Open file
				}
				if (ImGui::MenuItem("Save"))
				{
					ErmyProject::Instance().Save();
					//Save file
				}
				if (ImGui::MenuItem("Exit"))
				{
					gIsEditorRunning = false;
					//Exit
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Copy"))
				{
					//Copy
				}
				if (ImGui::MenuItem("Cut"))
				{
					//Cut
				}
				if (ImGui::MenuItem("Paste"))
				{
					//Paste
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Recompile Shaders"))
				{
					ErmyProject::Instance().RecompileAllInternalShaders();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Windows"))
			{
				if (ImGui::MenuItem("Assets Browser"))
				{
					editor::screen::AssetsBrowserScreen::Instance()->Show();
				}
				if (ImGui::MenuItem("Hierarchy"))
				{
					editor::screen::HierarchyScreen::Instance()->Show();
				}
				if (ImGui::MenuItem("Properties"))
				{
					editor::screen::PropertyEditorScreen::Instance()->Show();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void DrawStatusBar()
	{
		if (ImGui::BeginViewportSideBar("StatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, 32, ImGuiWindowFlags_NoDocking))
		{
			ImGui::End();
		}		
	}
}

void main_window::Draw()
{
	DrawMainMenu();
	DrawStatusBar();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiID dockspace_id = ImGui::GetID("ErmyDockspace");
	bool p_true = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace Demo", &p_true, window_flags);
	ImGui::PopStyleVar();
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	ImGui::PopStyleVar(2);

	editor::screen::EditorScreen::DrawAll();
	//Other screens
	ErmyProject::Instance().DrawProjectSettings();

	
	ImGui::End();
}
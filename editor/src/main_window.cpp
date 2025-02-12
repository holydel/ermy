#include "main_window.h"
#include <imgui.h>

#include "project/eproject.h"
#include "shader_compiler/shader_compiler.h"

extern bool gIsEditorRunning;

void main_window::Draw()
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

		ImGui::EndMainMenuBar();
	}

	ErmyProject::Instance().DrawProjectSettings();
}
#include "main_window.h"
#include <imgui.h>

#include "project/eproject.h"
#include "shader_compiler/shader_compiler.h"

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
				//Save file
			}
			if (ImGui::MenuItem("Exit"))
			{
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
				ErmyProject::Instance().RecompileAllShaders();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}
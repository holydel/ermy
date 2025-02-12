#include "screens/hierarchy_screen.h"
#include <imgui.h>

using namespace editor::screen;

void HierarchyScreen::Draw()
{
	if (ImGui::Begin("Hierarchy", &isShowed))
	{
        if (ImGui::TreeNode("Scene")) {
            ImGui::Text("This is the content of Node 1.");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Canvas")) {
            ImGui::Text("This is the content of Node 2.");
            ImGui::TreePop();
        }

		ImGui::End();
	}
}

HierarchyScreen* editor::screen::HierarchyScreen::Instance()
{
	static HierarchyScreen gHierarchyScreen;
	return &gHierarchyScreen;
}

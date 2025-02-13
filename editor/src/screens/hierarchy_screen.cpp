#include "screens/hierarchy_screen.h"
#include <imgui.h>

using namespace editor::screen;

void HierarchyScreen::Draw()
{
	if (ImGui::Begin("Heirarchy", &isShowed))
	{
		ImGui::End();
	}
}

HierarchyScreen* HierarchyScreen::Instance()
{
	static HierarchyScreen gHierarchyScreen;
	return &gHierarchyScreen;
}

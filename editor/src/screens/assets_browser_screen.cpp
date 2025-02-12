#include "screens/assets_browser_screen.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace editor::screen;

void AssetsBrowserScreen::Draw()
{
	if (ImGui::Begin("Assets Browser", &isShowed))
	{
		if (ImGui::TreeNode("assets")) {
			ImGui::Text("This is the content of Node 1.");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("shaders")) {
			ImGui::Text("This is the content of Node 2.");
			ImGui::TreePop();
		}

		ImGui::End();
	}	
}

AssetsBrowserScreen* AssetsBrowserScreen::Instance()
{
	static AssetsBrowserScreen gAssetsBrowserScreen;
	return &gAssetsBrowserScreen;
}

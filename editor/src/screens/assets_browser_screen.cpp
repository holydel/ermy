#include "screens/assets_browser_screen.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <vector>
#include <project/eproject.h>
#include "screens/property_editor_screen.h"

namespace fs = std::filesystem;
using namespace editor::screen;

AssetFolder* gSelectedFolder = nullptr;
Asset* gSelectedAsset = nullptr;

void DrawAssetsFolders(AssetFolder *folder)
{
    // Check if the current folder is selected
    bool isSelected = (gSelectedFolder == folder);

    // Use ImGuiTreeNodeFlags_Selected to highlight the selected folder
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (isSelected)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // Use TreeNodeEx to draw the folder with the selected state
    bool isOpen = ImGui::TreeNodeEx(folder->NameCStr(), nodeFlags);

    // Handle click events to update the selected folder
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        gSelectedFolder = folder;
    }

    // Recursively draw subdirectories if the folder is open
    if (isOpen)
    {
        for (auto f : folder->subdirectories)
        {
            DrawAssetsFolders(f);
        }

        ImGui::TreePop();
    }
}

void DrawFolderContents(AssetFolder* folder)
{
    if (!folder)
        return;

    // Display the contents of the selected folder
    ImGui::Text("Contents of Folder: %s", folder->NameCStr());
    ImGui::Separator();

    // Example: Display files or assets in the folder
    for (const auto& file : folder->content)
    {
        // Check if the current file is selected
        bool isSelected = (gSelectedAsset == file);

        // Use ImGui::Selectable to make the file name clickable and selectable
        if (ImGui::Selectable(file->NameCStr(), isSelected))
        {
            // Update the current file when clicked
            gSelectedAsset = file;

            if (!gSelectedAsset->GetData())
                gSelectedAsset->Import();

            if (auto data = gSelectedAsset->GetData())
            {
                PropertyEditorScreen::Instance()->SetProps(data);
            }
        }

        //ImGui::Text("%s", file->NameCStr());
    }
}

void AssetsBrowserScreen::Draw()
{
	if (ImGui::Begin("Assets Browser", &isShowed))
	{
        ImGui::Columns(2, "AssetBrowserColumns", true);

        ImGui::BeginChild("FoldersColumn", ImVec2(0, 0), true);
		DrawAssetsFolders(ErmyProject::Instance().GetRootAssets());			
		DrawAssetsFolders(ErmyProject::Instance().GetRootShaders());
        ImGui::EndChild();

        ImGui::NextColumn();

        ImGui::BeginChild("ContentsColumn", ImVec2(0, 0), true);
        DrawFolderContents(gSelectedFolder);
        ImGui::EndChild();

        ImGui::Columns(1);

		ImGui::End();
	}	


}

AssetsBrowserScreen* AssetsBrowserScreen::Instance()
{
	static AssetsBrowserScreen gAssetsBrowserScreen;
	return &gAssetsBrowserScreen;
}

#include "property_editor_screen.h"

void editor::screen::PropertyEditorScreen::Draw()
{
	if (ImGui::Begin("Property", &isShowed))
	{

		ImGui::End();
	}
}

PropertyEditorScreen* editor::screen::PropertyEditorScreen::Instance()
{
	static AssetsBrowserScreen gAssetsBrowserScreen;
	return &gAssetsBrowserScreen;
}

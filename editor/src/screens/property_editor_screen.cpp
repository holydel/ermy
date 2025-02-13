#include "screens/property_editor_screen.h"
#include <imgui.h>

using namespace editor::screen;
void PropertyEditorScreen::Draw()
{
	if (ImGui::Begin("Property", &isShowed))
	{
		if (props)
		{
			props->DrawProps();
		}

		ImGui::End();
	}
}

PropertyEditorScreen* PropertyEditorScreen::Instance()
{
	static PropertyEditorScreen gPropertyEditorScreen;
	return &gPropertyEditorScreen;
}


void PropertyEditorScreen::SetProps(Props* props)
{
	this->props = props;
}

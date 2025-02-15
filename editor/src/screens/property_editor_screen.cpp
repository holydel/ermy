#include "screens/property_editor_screen.h"
#include <imgui.h>
#include "preview_renderer.h"
#include <algorithm>

using namespace editor::screen;

PropertyEditorScreen::PropertyEditorScreen()
{
	PreviewRenderer::Instance();
}

PropertyEditorScreen::~PropertyEditorScreen()
{
	
}

void PropertyEditorScreen::Draw()
{
	if (ImGui::Begin("Property", &isShowed))
	{
		if (props)
		{
			props->DrawProps();
			int size = std::min(512.0f,ImGui::GetContentRegionAvail().x);

			ImGui::Image(PreviewRenderer::Instance().GetPreviewTexture(), ImVec2(size, size));
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

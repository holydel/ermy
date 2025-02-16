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
            if (ImGui::Button("Reset View"))
            {
                PreviewRenderer::Instance().ResetView();
            }
            ImVec2 image_pos = ImGui::GetCursorScreenPos();
            ImVec2 image_size(size, size);
            
            
			ImGui::Image(PreviewRenderer::Instance().GetPreviewTexture(), image_size);
            ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
            // Check if the mouse is hovering over the image
            bool is_hovered = ImGui::IsItemHovered();

            // Handle mouse wheel for zooming
            if (is_hovered)
            {
                float zoom_speed = 0.1f; // Adjust zoom speed as needed
                
                float zoom_delta = ImGui::GetIO().MouseWheel;

                if (zoom_delta != 0.0f)
                {
                    // Adjust your zoom level here
                    // For example, you might have a zoom factor that you adjust:
                    float zoom_factor = 1.0f + zoom_delta * zoom_speed;
                    
                    PreviewRenderer::Instance().MouseZoom(zoom_factor);
                   // ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
                    // Apply the zoom factor to your image or viewport
                }
            }

            // Handle mouse down/up/move events
            if (is_hovered)
            {
                if (ImGui::IsMouseClicked(0)) // Left mouse button down
                {
                    // Handle mouse down event
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    ImVec2 relative_mouse_pos = ImVec2((mouse_pos.x - image_pos.x)/image_size.x, (mouse_pos.y - image_pos.y)/image_size.y);
                    // Do something with the relative mouse position

                    PreviewRenderer::Instance().MouseDown(relative_mouse_pos.x, relative_mouse_pos.y);
                }

                if (ImGui::IsMouseReleased(0)) // Left mouse button up
                {
                    PreviewRenderer::Instance().MouseUp();
                    // Handle mouse up event
                }

                if (ImGui::IsMouseDragging(0)) // Left mouse button drag
                {
                    // Handle mouse move/drag event
                    ImVec2 drag_delta = ImGui::GetMouseDragDelta(0);
                    ImVec2 relative_mouse_drag = ImVec2(drag_delta.x / image_size.x, drag_delta.y / image_size.y);
                    PreviewRenderer::Instance().MouseMove(relative_mouse_drag.x, relative_mouse_drag.y);
                    // Do something with the drag delta
                }
            }
            else
            {
                PreviewRenderer::Instance().MouseUp();
            }
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

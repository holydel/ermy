#pragma once
#include "editor_screen.h"

namespace editor::screen
{
	class PropertyEditorScreen : public EditorScreen
	{
	public:
		void Draw() override;
		static PropertyEditorScreen* Instance();
	};
}
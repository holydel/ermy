#pragma once
#include "editor_screen.h"

namespace editor::screen
{
	class HierarchyScreen : public EditorScreen
	{

	public:
		void Draw() override;
		static HierarchyScreen* Instance();

	};
}


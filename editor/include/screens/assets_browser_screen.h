#pragma once
#include "editor_screen.h"
#include "assets/asset.h"
#include <string>

namespace editor::screen
{
	class AssetsBrowserScreen : public EditorScreen
	{
	public:
		void Draw() override;
		static AssetsBrowserScreen* Instance();
	};
}


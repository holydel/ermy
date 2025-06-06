#pragma once
#include "editor_screen.h"
#include "assets/asset.h"
#include <string>

namespace editor::screen
{
	class AssetsBrowserScreen : public EditorScreen
	{
		bool firstFrame = true;
	public:
		void Draw() override;
		static AssetsBrowserScreen* Instance();
	};
}


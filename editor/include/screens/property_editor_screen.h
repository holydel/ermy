#pragma once
#include "editor_screen.h"
#include "props.h"

namespace editor::screen
{
	class PropertyEditorScreen : public EditorScreen
	{
		Props* props = nullptr;
		PropertyEditorScreen();
		~PropertyEditorScreen();
	public:
		void Draw() override;
		static PropertyEditorScreen* Instance();


		void SetProps(Props* props);
	};
}
#include "screens/editor_screen.h"
#include <vector>
using namespace editor::screen;

std::vector<EditorScreen*> gAllScreens;

EditorScreen::EditorScreen()
{
	gAllScreens.push_back(this);
}

void EditorScreen::DrawAll()
{
	for (auto s : gAllScreens)
	{
		s->Draw();
	}
}
#pragma once
#include <ermy_api.h>

namespace editor
{
	namespace screen
	{
		class EditorScreen
		{
		protected:
			bool isShowed = false;
		public:
			EditorScreen();
			virtual ~EditorScreen()
			{

			}

			void Show()
			{
				isShowed = true;
			}
			void Hide()
			{
				isShowed = false;
			}
			bool IsShowed()
			{
				return isShowed;
			}
			
			virtual void Draw()
			{

			}
			static void DrawAll();
		};
	}
}
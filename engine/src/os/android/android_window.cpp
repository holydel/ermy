#include "android_utils.h"
#include "../os.h"
#include <ermy_input.h>

ANativeWindow* gMainWindow = nullptr;

void* os::CreateNativeWindow()
{
	return nullptr;
}

bool os::Update()
{

	return true;
}

void* os::GetNativeWindowHandle()
{
	return static_cast<void*>(gMainWindow);
}

extern int androidCursorX;
extern int androidCursorY;

glm::vec2 ermy::input::mouse::GetCurrentPosition()
{
	return { androidCursorX ,androidCursorY };
}
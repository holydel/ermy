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

glm::ivec2 ermy::input::mouse::GetCurrentPosition()
{
	return { 0 ,0 };
}
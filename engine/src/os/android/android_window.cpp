#include "android_utils.h"
#include "../os.h"

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
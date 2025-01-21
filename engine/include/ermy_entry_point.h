#pragma once
#include "ermy_api.h"

#ifdef ERMY_OS_ANDROID

#include <android/native_activity.h>

void DummyLinkerExportFunctionForAndroidMain()
{
	ANativeActivity* activity = nullptr;
	void* savedState = nullptr;
	size_t savedStateSize = 0;
	ANativeActivity_onCreate(activity, savedState, savedStateSize);
}

#endif
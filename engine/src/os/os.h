#pragma once

#include "os_entry_point.h"
#include "os_utils.h"

namespace os
{
	void* CreateNativeWindow();
	bool Update();

	void* GetNativeWindowHandle();
	void* GetAppInstanceHandle();

}
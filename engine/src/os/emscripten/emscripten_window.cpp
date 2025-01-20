#pragma once
#include "emscripten_utils.h"
#include "application.h"
#include "../ermy_os.h"


void* os::CreateNativeWindow()
{
    return nullptr;
}

bool os::Update()
{
	return true;
}
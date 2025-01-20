#pragma once
#include "../rendering_interface.h"
#include "../../os/emscripten/emscripten_utils.h"

extern WGPUDevice gDevice;
extern WGPUAdapter gAdapter;
extern WGPUInstance gInstance;
extern WGPUQueue gQueue ;
extern WGPUSurface gSurface;
extern WGPUSwapChain  gSwapChain;
extern WGPUTextureFormat gPrefferedBackbufferFormat;
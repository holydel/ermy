#pragma once
#include "../rendering_interface.h"
#include "ermy_vulkan.h"

extern VkInstance gVKInstance;
extern VkAllocationCallbacks* gVKGlobalAllocationsCallbacks;
extern VkDebugUtilsMessengerEXT gVKDebugMessenger;
#pragma once
#include "../rendering_interface.h"
#include "ermy_vulkan.h"

extern VkInstance gVKInstance;
extern VkAllocationCallbacks* gVKGlobalAllocationsCallbacks;
extern VkDebugUtilsMessengerEXT gVKDebugMessenger;
extern VkPhysicalDevice gVKPhysicalDevice;

extern VkDevice gVKDevice;
extern VmaAllocator gVMA_Allocator;
extern VkQueue gVKMainQueue;
extern VkQueue gVKComputeAsyncQueue;
extern VkQueue gVKTransferAsyncQueue;
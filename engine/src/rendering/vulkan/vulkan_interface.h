#pragma once
#include "../rendering_interface.h"
#include "ermy_vulkan.h"
#include <ermy_application.h>

extern VkInstance gVKInstance;
extern VkAllocationCallbacks* gVKGlobalAllocationsCallbacks;
extern VkDebugUtilsMessengerEXT gVKDebugMessenger;
extern VkPhysicalDevice gVKPhysicalDevice;

extern VkDevice gVKDevice;
extern VmaAllocator gVMA_Allocator;
extern VkQueue gVKMainQueue;
extern VkQueue gVKComputeAsyncQueue;
extern VkQueue gVKTransferAsyncQueue;
extern VkRenderPass gVKRenderPass;

extern ermy::Application::StaticConfig::VKConfig gVKConfig;

struct DeviceEnabledExtensions
{
	bool NvDecompressMemory : 1 = false;
	bool KhrBufferDeviceAddress : 1 = false;
	bool KhrDedicatedAllocation : 1 = false;
	bool KhrSynchronization2 : 1 = false;
	bool KhrTimelineSemaphore : 1 = false;
	bool ExtMemoryPriority : 1 = false;
	bool ExtPageableDeviceLocalMemory : 1 = false;
	bool KhrDynamicRendering : 1 = false;
	bool KhrDynamicRenderingLocalRead : 1 = false;
};

extern DeviceEnabledExtensions gVKDeviceEnabledExtensions;
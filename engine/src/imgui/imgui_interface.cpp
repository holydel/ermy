#include <ermy_api.h>

#ifdef ERMY_GAPI_VULKAN
#include "../rendering/vulkan/vulkan_interface.h"
#include "../rendering/vulkan/vulkan_swapchain.h"
static void ermy_check_vk_result(VkResult err)
{
}
#endif

#ifdef ERMY_GAPI_D3D12
#include "../rendering/d3d12/d3d12_interface.h"
static ID3D12DescriptorHeap* gImgui_pd3dSrvDescHeap = nullptr;
#endif

#include "ermy_imgui.h"
#include "imgui_interface.h"
#include "../os/os.h"
#include "application.h"
#include "../os/os.h"

void imgui_interface::Initialize()
{
	auto& app = GetApplication();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
	if (app.staticConfig.imgui.enableDocking)
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch	

	ImGui::StyleColorsDark();

#ifdef ERMY_GAPI_VULKAN
	extern VkSampleCountFlagBits gNumSamplesMSAA;

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = gVKInstance;
	init_info.PhysicalDevice = gVKPhysicalDevice;
	init_info.Device = gVKDevice;
	init_info.QueueFamily = 0;
	init_info.Queue = gVKMainQueue;
	init_info.PipelineCache = VK_NULL_HANDLE; //TODO: implement pipeline cache
	init_info.Allocator = nullptr;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.CheckVkResultFn = ermy_check_vk_result;
	init_info.MSAASamples = gVKSurfaceSamples;
	init_info.RenderPass = gVKRenderPass;
	init_info.MinAllocationSize = 1024 * 1024;

	init_info.UseDynamicRendering = gVKConfig.useDynamicRendering;
	if (gVKConfig.useDynamicRendering)
	{
		VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
		pipeline_rendering_create_info.colorAttachmentCount = 1;
		pipeline_rendering_create_info.pColorAttachmentFormats = &gVKSurfaceFormat;
		init_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;
	}
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo dp_create_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,nullptr };
	dp_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	dp_create_info.maxSets = 1000;
	dp_create_info.pPoolSizes = pool_sizes;
	dp_create_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);

	vkCreateDescriptorPool(gVKDevice, &dp_create_info, nullptr, &init_info.DescriptorPool);

	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();
#endif

#ifdef ERMY_GAPI_D3D12
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 20;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	gD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gImgui_pd3dSrvDescHeap));

	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.CommandQueue = gD3DCommandQueue;
	init_info.Device = gD3DDevice;	
	init_info.SrvDescriptorHeap = gImgui_pd3dSrvDescHeap;
	init_info.NumFramesInFlight = 3;
	init_info.RTVFormat = gD3DSwapChainFormat;
	init_info.LegacySingleSrvCpuDescriptor = gImgui_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
	init_info.LegacySingleSrvGpuDescriptor = gImgui_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
	ImGui_ImplDX12_Init(&init_info);

	ImGui_ImplDX12_CreateDeviceObjects();
	//ImGui_ImplDX12_CreateFontsTexture();
#endif

#ifdef ERMY_GAPI_WEBGPU
	ImGui_ImplWGPU_InitInfo initInfo = {};
	initInfo.Device = gDevice;
	initInfo.RenderTargetFormat = gPrefferedBackbufferFormat;
	initInfo.NumFramesInFlight = 3;
	initInfo.DepthStencilFormat = WGPUTextureFormat::WGPUTextureFormat_Undefined;

	ImGui_ImplWGPU_Init(&initInfo);
#endif


#ifdef ERMY_OS_WINDOWS 
	HWND mainWinHandle = static_cast<HWND>(os::GetNativeWindowHandle());
	ImGui_ImplWin32_Init(mainWinHandle);
#endif
#ifdef ERMY_OS_LINUX
	auto win = static_cast<xcb_window_t*>(mercury::platform::getMainWindowHandle());
	auto connection = static_cast<xcb_connection_t*>(mercury::platform::getAppInstanceHandle());
	ImGui_ImplX11_Init(connection, win);
#endif
#ifdef ERMY_OS_ANDROID
	ImGui_ImplAndroid_Init(static_cast<ANativeWindow*>(os::GetNativeWindowHandle()));
	ImGui::GetIO().FontGlobalScale = 2.0f;
#endif
#ifdef ERMY_OS_MACOS
	void* view = mercury::platform::getAppInstanceHandle(); //view
	ImGui_ImplOSX_Init(view);
#endif
#ifdef ERMY_OS_EMSCRIPTEN
	ImGui_ImplEmscripten_Init();
#endif
}

void imgui_interface::Shutdown()
{
#ifdef ERMY_GAPI_VULKAN
		ImGui_ImplVulkan_Shutdown();
#endif
#ifdef ERMY_GAPI_D3D12
		ImGui_ImplDX12_Shutdown();
#endif
#ifdef ERMY_GAPI_WEBGPU
		ImGui_ImplWGPU_Shutdown();
#endif
#ifdef ERMY_OS_ANDROID
		ImGui_ImplAndroid_Shutdown();
#endif
#ifdef ERMY_OS_WINDOWS
		ImGui_ImplWin32_Shutdown();
#endif
#ifdef ERMY_OS_EMSCRIPTEN
		ImGui_ImplEmscripten_Shutdown();
#endif

	ImGui::DestroyContext();
}

void imgui_interface::BeginFrame(void* cmdList)
{
#ifdef ERMY_GAPI_VULKAN
	ImGui_ImplVulkan_NewFrame();
#endif
#ifdef ERMY_GAPI_D3D12
	ImGui_ImplDX12_NewFrame();
#endif
#ifdef ERMY_GAPI_WEBGPU
	ImGui_ImplWGPU_NewFrame();
#endif
#ifdef ERMY_OS_WINDOWS 
	ImGui_ImplWin32_NewFrame();
#endif
#ifdef ERMY_OS_LINUX

	ImGui_ImplX11_NewFrame();
#endif
#ifdef ERMY_OS_ANDROID
	ImGui_ImplAndroid_NewFrame();
#endif
#ifdef ERMY_OS_MACOS
	void* view = mercury::platform::getAppInstanceHandle(); //view
	ImGui_ImplOSX_NewFrame(view);
#endif
#ifdef ERMY_OS_EMSCRIPTEN
	ImGui_ImplEmscripten_Event();
	ImGui_ImplEmscripten_NewFrame();
#endif
	ImGui::NewFrame();
}

void imgui_interface::EndFrame(void* cmdList)
{
	ImGui::ShowDemoWindow(); // Show demo window! :)
	ImGui::Render();

#ifdef ERMY_GAPI_VULKAN
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(cmdList));
#endif

#ifdef ERMY_GAPI_D3D12
	auto cmdListD3D12 = static_cast<ID3D12GraphicsCommandList*>(cmdList);

	cmdListD3D12->SetDescriptorHeaps(1, &gImgui_pd3dSrvDescHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdListD3D12);
#endif
#ifdef ERMY_GAPI_WEBGPU
	auto cmdList = static_cast<WGPURenderPassEncoder>(ctx.impl);
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), cmdList);
#endif
}

void imgui_interface::Render()
{

}
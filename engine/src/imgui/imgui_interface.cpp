﻿#include <ermy_api.h>

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
#include <ermy_input.h>
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

void DrawGamePad(ImDrawList* draw_list, const ermy::input::gamepad::GamePadState& state, bool enabled, float battery_fraction) {
	float x = ImGui::GetCursorScreenPos().x;
	float y = ImGui::GetCursorScreenPos().y;

	// Define positions for gamepad elements
	// Shoulder buttons (LB and RB)
	ImVec2 lb_min = ImVec2(x + 20, y + 10);
	ImVec2 lb_max = ImVec2(x + 60, y + 20);
	ImVec2 rb_min = ImVec2(x + 140, y + 10);
	ImVec2 rb_max = ImVec2(x + 180, y + 20);

	// D-pad (Up, Down, Left, Right)
	ImVec2 dpad_up_min = ImVec2(x + 40, y + 40);
	ImVec2 dpad_up_max = ImVec2(x + 60, y + 60);
	ImVec2 dpad_down_min = ImVec2(x + 40, y + 60);
	ImVec2 dpad_down_max = ImVec2(x + 60, y + 80);
	ImVec2 dpad_left_min = ImVec2(x + 20, y + 60);
	ImVec2 dpad_left_max = ImVec2(x + 40, y + 80);
	ImVec2 dpad_right_min = ImVec2(x + 60, y + 60);
	ImVec2 dpad_right_max = ImVec2(x + 80, y + 80);

	// Face buttons (X, Y, A, B)
	float button_radius = 5.0f;
	ImVec2 face_x_center = ImVec2(x + 140, y + 60); // Left
	ImVec2 face_y_center = ImVec2(x + 150, y + 50); // Top
	ImVec2 face_a_center = ImVec2(x + 150, y + 70); // Bottom
	ImVec2 face_b_center = ImVec2(x + 160, y + 60); // Right

	// Analog sticks
	float stick_radius = 20.0f;
	ImVec2 left_stick_base = ImVec2(x + 50, y + 100);
	ImVec2 right_stick_base = ImVec2(x + 150, y + 100);

	// Triggers
	ImVec2 lt_bar_min = ImVec2(x + 20, y + 130);
	ImVec2 lt_bar_max = ImVec2(x + 80, y + 140);
	ImVec2 rt_bar_min = ImVec2(x + 120, y + 130);
	ImVec2 rt_bar_max = ImVec2(x + 180, y + 140);

	// Back and Start buttons
	ImVec2 back_min = ImVec2(x + 80, y + 50);
	ImVec2 back_max = ImVec2(x + 90, y + 60);
	ImVec2 start_min = ImVec2(x + 110, y + 50);
	ImVec2 start_max = ImVec2(x + 120, y + 60);

	// Stick press buttons (LS and RS)
	ImVec2 ls_center = ImVec2(x + 50, y + 120);
	ImVec2 rs_center = ImVec2(x + 150, y + 120);

	// Define colors
	ImU32 color_button_normal = IM_COL32(100, 100, 100, 255);   // Dark gray
	ImU32 color_button_pressed = IM_COL32(0, 255, 0, 255);      // Green
	ImU32 color_stick_base = IM_COL32(50, 50, 50, 255);         // Medium gray
	ImU32 color_stick_position = IM_COL32(255, 0, 0, 255);      // Red
	ImU32 color_trigger_bar = IM_COL32(200, 200, 200, 255);     // Light gray
	ImU32 color_trigger_fill = IM_COL32(0, 0, 255, 255);        // Blue
	ImU32 color_disabled = IM_COL32(50, 50, 50, 255);           // Dark gray for disabled

	// Adjust state based on enabled flag
	ermy::input::gamepad::GamePadState draw_state = state;
	if (!enabled) {
		draw_state.leftStick = glm::vec2(0, 0);
		draw_state.rightStick = glm::vec2(0, 0);
		draw_state.leftTrigger = 0.0f;
		draw_state.rightTrigger = 0.0f;
		draw_state.A = draw_state.B = draw_state.X = draw_state.Y = false;
		draw_state.LB = draw_state.RB = draw_state.Back = draw_state.Start = false;
		draw_state.LS = draw_state.RS = draw_state.Up = draw_state.Down = false;
		draw_state.Left = draw_state.Right = false;
	}

	// Draw shoulder buttons
	draw_list->AddRectFilled(lb_min, lb_max, enabled && draw_state.LB ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddRectFilled(rb_min, rb_max, enabled && draw_state.RB ? color_button_pressed : (enabled ? color_button_normal : color_disabled));

	// Draw D-pad
	draw_list->AddRectFilled(dpad_up_min, dpad_up_max, enabled && draw_state.Up ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddRectFilled(dpad_down_min, dpad_down_max, enabled && draw_state.Down ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddRectFilled(dpad_left_min, dpad_left_max, enabled && draw_state.Left ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddRectFilled(dpad_right_min, dpad_right_max, enabled && draw_state.Right ? color_button_pressed : (enabled ? color_button_normal : color_disabled));

	// Draw face buttons
	draw_list->AddCircleFilled(face_x_center, button_radius, enabled && draw_state.X ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddCircleFilled(face_y_center, button_radius, enabled && draw_state.Y ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddCircleFilled(face_a_center, button_radius, enabled && draw_state.A ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddCircleFilled(face_b_center, button_radius, enabled && draw_state.B ? color_button_pressed : (enabled ? color_button_normal : color_disabled));

	// Draw analog sticks
	draw_list->AddCircleFilled(left_stick_base, stick_radius, color_stick_base);
	ImVec2 left_stick_pos = ImVec2(left_stick_base.x + draw_state.leftStick.x * (stick_radius - 5), left_stick_base.y - draw_state.leftStick.y * (stick_radius - 5));
	draw_list->AddCircleFilled(left_stick_pos, 5.0f, color_stick_position);

	draw_list->AddCircleFilled(right_stick_base, stick_radius, color_stick_base);
	ImVec2 right_stick_pos = ImVec2(right_stick_base.x + draw_state.rightStick.x * (stick_radius - 5), right_stick_base.y - draw_state.rightStick.y * (stick_radius - 5));
	draw_list->AddCircleFilled(right_stick_pos, 5.0f, color_stick_position);

	// Draw triggers
	draw_list->AddRect(lt_bar_min, lt_bar_max, color_trigger_bar);
	float lt_fill_width = (lt_bar_max.x - lt_bar_min.x) * draw_state.leftTrigger;
	draw_list->AddRectFilled(lt_bar_min, ImVec2(lt_bar_min.x + lt_fill_width, lt_bar_max.y), color_trigger_fill);

	draw_list->AddRect(rt_bar_min, rt_bar_max, color_trigger_bar);
	float rt_fill_width = (rt_bar_max.x - rt_bar_min.x) * draw_state.rightTrigger;
	draw_list->AddRectFilled(rt_bar_min, ImVec2(rt_bar_min.x + rt_fill_width, rt_bar_max.y), color_trigger_fill);

	// Draw Back and Start buttons
	draw_list->AddRectFilled(back_min, back_max, enabled && draw_state.Back ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddRectFilled(start_min, start_max, enabled && draw_state.Start ? color_button_pressed : (enabled ? color_button_normal : color_disabled));

	// Draw stick press buttons (LS and RS)
	draw_list->AddCircleFilled(ls_center, button_radius, enabled && draw_state.LS ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	draw_list->AddCircleFilled(rs_center, button_radius, enabled && draw_state.RS ? color_button_pressed : (enabled ? color_button_normal : color_disabled));
	
	// Draw battery progress bar
	ImVec2 progress_bar_min = ImVec2(x + 50, y + 150);
	ImVec2 progress_bar_max = ImVec2(x + 150, y + 160);
	draw_list->AddRectFilled(progress_bar_min, progress_bar_max, IM_COL32(200, 200, 200, 255)); // Light gray background

	if (enabled) {
		float fill_width = (progress_bar_max.x - progress_bar_min.x) * battery_fraction;
		ImVec2 fill_max = ImVec2(progress_bar_min.x + fill_width, progress_bar_max.y);
		ImU32 fill_color;
		if (battery_fraction > 0.75f) {
			fill_color = IM_COL32(0, 255, 0, 255); // Green for high battery
		}
		else if (battery_fraction > 0.25f) {
			fill_color = IM_COL32(255, 255, 0, 255); // Yellow for medium battery
		}
		else {
			fill_color = IM_COL32(255, 0, 0, 255); // Red for low battery
		}
		draw_list->AddRectFilled(progress_bar_min, fill_max, fill_color);

		// Add battery level text
		const char* level_str;
		if (battery_fraction >= 1.0f) {
			level_str = "Full";
		}
		else if (battery_fraction >= 0.5f) {
			level_str = "Medium";
		}
		else if (battery_fraction >= 0.25f) {
			level_str = "Low";
		}
		else {
			level_str = "Empty";
		}
		draw_list->AddText(ImVec2(x + 155, y + 150), IM_COL32(255, 255, 255, 255), level_str);
	}
	else {
		draw_list->AddText(ImVec2(x + 155, y + 150), IM_COL32(255, 255, 255, 255), "N/A");
	}
	
	ImGui::Dummy(ImVec2(210, 160)); // Dummy space to avoid overlap with other UI elements
}

void DrawStatisticsWindow()
{
	ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Separator();
	ImGui::Text("OS: %s", ermy::os_utils::GetOSName());
	ImGui::SameLine();
	ImGui::Text("GAPI: %s", rendering_interface::GetName());
	ImGui::Separator();
	ImGui::Text("Processor: some cpu");
	ImGui::Text("GPU: some gpu");
	ImGui::Text("GPU: some gpu");
	ImGui::Text("RAM: some ram");
	ImGui::Text("Disk: some storage info");
	ImGui::Separator();

	if (ImGui::BeginTabBar("SystemTabs")) {
		if (ImGui::BeginTabItem("CPU")) {
			ImGui::Text("CPU Settings");
			ImGui::Text("Core Count: 8");
			ImGui::Text("Clock Speed: 3.6 GHz");
			///ImGui::SliderFloat("CPU Usage", &cpu_usage, 0.0f, 100.0f, "%.1f%%");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Video")) {
			ImGui::Text("Video Settings");
			ImGui::Text("Resolution: 1920x1080");
			//ImGui::Checkbox("VSync", &vsync_enabled);
			//ImGui::ColorEdit3("Background Color", background_color);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sound")) {
			ImGui::Text("Sound Settings");
			//ImGui::SliderFloat("Volume", &volume, 0.0f, 100.0f, "%.1f%%");
			///ImGui::Checkbox("Mute", &mute_enabled);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Input")) {
			ImGui::Text("Input Settings");
			ImGui::Text("Connected Devices: Keyboard, Mouse");
			//ImGui::Checkbox("Gamepad Support", &gamepad_enabled);

			for (int i = 0; i < 4; ++i)
			{
				DrawGamePad(ImGui::GetWindowDrawList(), ermy::input::gamepad::GetState(i), ermy::input::gamepad::IsConnected(i),float(ermy::input::gamepad::GetControllerBatteryLevel(i)) / 255.0f);
				ImGui::SameLine();
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void imgui_interface::EndFrame(void* cmdList)
{
	//ImGui::ShowDemoWindow(); // Show demo window! :)
	//DrawStatisticsWindow();
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
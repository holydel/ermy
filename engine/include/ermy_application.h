#pragma once
#include <string>
#include "ermy_api.h"
#include "ermy_commandlist.h"

namespace ermy
{
	class Application
	{
	private:

	public:
		struct StaticConfig
		{
			enum class OutputMode
			{
				Headless,
				VR,
				Window, //default - also borderless fullscreen window 
				Display
			};

			struct WindowConfig
			{
				enum class WindowMode
				{
					Windowed,
					BorderlessFullscreen,
					ExclusiveFullscreen //tried to eliminate the compositor's delay
				};

				int width = 800;
				int height = 600;
				std::string title = ""; //when e,pty - get name from AppConfig
				WindowMode mode = WindowMode::Windowed;

			};

			struct SwapchainConfig
			{
				enum ColorWidth
				{
					Prefer16bit,
					Prefer32bit,
					PreferHDR
				};

				ColorWidth colorWidth = ColorWidth::Prefer32bit;

				enum VSyncMode
				{
					NoVSync,
					AdaptiveVSync,
					AlwaysVSync
				};

				VSyncMode vsync = VSyncMode::AlwaysVSync;

				bool useAlpha = false;
			};

			struct ImguiConfig
			{
				bool enable  = true;
				bool enableDocking  = false;
			};

			struct EngineConfig
			{
				bool EnableIMGUI = false;
				bool EnableSound = false;
				bool EnableBulletPhysics = false;
				bool EnableXR = false;

				bool GraphicsValidation  = false;
				bool XRValidation  = false;				
			};

			struct D3D12Config
			{
				bool useWorkGraphs = false;
			};

			struct VKConfig
			{
				bool useDynamicRendering = false;
			};

			struct RendererConfig
			{
#ifdef ERMY_GAPI_VULKAN
				VKConfig vkConfig;
#endif
#ifdef ERMY_GAPI_D3D12
				D3D12Config d3d12Config;
#endif

				i8 adapterID = -1;

#ifdef NDEBUG
				bool enableDebugLayers = false;
#else
				bool enableDebugLayers = true;
#endif

				bool enableRaytracing : 1 = false;	
				bool enableBarycentricFS : 1 = false;
				bool enableMeshShader : 1 = false;
				bool enableSamplerFeedback : 1 = false;
				bool enableVariableRateShading : 1 = false;
				bool enableGeometryShader : 1 = false;
				bool enableTessellation : 1 = false;
				bool enableSamplerYCbCr : 1 = false;
			};

			OutputMode outputMode = OutputMode::Window;
			WindowConfig window;
			EngineConfig engine;
			RendererConfig render;
			ImguiConfig imgui;
			SwapchainConfig swapchain;

			struct Version
			{
				union
				{
					struct
					{
						u8 major;
						u8 minor;
						u16 patch;
					};

					u32 packed;
				};
			} version = Version{ {{0,0,1}} };

			std::string appName = "Unnamed Ermy APP";

			const std::string& GetWindowTitle()
			{
				if (window.title.empty())
					return appName;
				else
					return window.title;
			}
		} staticConfig;

		Application();
		virtual ~Application();
	
		/// <summary>
		/// configure application - before engine initialization
		/// </summary>
		virtual void OnConfigure() {}

		/// <summary>
		/// initialize application - after engine initialization
		/// </summary>
		virtual void OnInitialization() {}

		/// <summary>
		/// Load scene data - after engine load prebuilt data.
		/// can be async to Begin/End Frame
		/// </summary>
		virtual void OnLoad() {}

		/// <summary>
		/// Begin Frame (after acquire frame from swapchain)
		/// </summary>
		virtual void OnBeginFrame(rendering::CommandList& finalCL) {}

		/// <summary>
		/// Begin Final Pass (after finall pass begin)
		/// </summary>
		virtual void OnBeginFinalPass(rendering::CommandList& finalCL) {}

		/// <summary>
		/// End frame (after submission to swapchain)
		/// </summary>
		virtual void OnEndFrame() {}

		/// <summary>
		/// After unload scene data
		/// </summary>
		virtual void OnUnLoad() {}

		/// <summary>
		/// just before engine deinitialization
		/// </summary>
		virtual void OnShutdown() {}

		virtual void OnIMGUI() {}

		virtual void OnApplicationClose() {}

		virtual bool IsRunning() {
			return true;
		}

		static const Application* GetApplication();
	};
}
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

				enum class InitialState
				{
					Normal,
					Maximized,
					Minimized,
					Hidden
				};

				int width = 800;
				int height = 600;
				std::string title = ""; //when empty - get name from AppConfig
				WindowMode mode = WindowMode::Windowed;
				InitialState initialState = InitialState::Normal;

				bool supportTransparent = false;

				bool isFullScreen() const
				{
					return mode == WindowMode::ExclusiveFullscreen || mode == WindowMode::BorderlessFullscreen;
				}
			};

			struct SwapchainConfig
			{
				enum class ColorWidth : u8
				{
					Prefer16bit,
					Prefer32bit,
					PreferHDR
				};

				enum class DepthMode : u8
				{
					None,
					Depth16,
					Depth24_Stencil8,
					Depth32F,
					Depth32F_Stencil8
				};

				enum class MSAAMode : u8
				{
					None = 1,
					Samples2 = 2,
					Samples4 = 4,
					Samples8 = 8,
					Samples16 = 16
				};

				ColorWidth colorWidth = ColorWidth::Prefer32bit;
				DepthMode depthMode = DepthMode::None;
				MSAAMode msaaMode = MSAAMode::None;

				enum class VSyncMode : u8
				{
					NoVSync,
					AdaptiveVSync,
					AlwaysVSync
				};

				VSyncMode vsync = VSyncMode::AlwaysVSync;

				bool useAlpha : 1 = false;
				bool tripleBuffering : 1 = true; //for VR - use double buffering
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
				bool EnableXRMirroring = false;
			};

			struct D3D12Config
			{
				bool useWorkGraphs = false;
			};

			struct VKConfig
			{
				bool useDynamicRendering = false;
			};

			struct XRConfig
			{
				bool useDebugLayers = true;
			};

			struct InputConfig
			{
				u16 checkGamepadIntervalMS = 250; //for one gamepad per tick
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
			InputConfig inputConfig;
			XRConfig xr;
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

			bool HasOutputWindow() const
			{
				if (outputMode == OutputMode::Window)
					return true;

				if (outputMode == OutputMode::VR)
				{
					if (engine.EnableXRMirroring)
						return true;
				}

				return false;
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

		virtual void OnUpdate() {}

		virtual void OnRender() {}
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
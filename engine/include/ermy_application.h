#pragma once
#include <string>
#include "ermy_api.h"

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

			struct EngineConfig
			{
				bool EnableIMGUI = false;
				bool EnableSound = false;
				bool EnableBulletPhysics = false;
				bool EnableXR = false;

				bool GraphicsValidation = false;
				bool XRValidation = false;
			};

			struct RendererConfig
			{
				i8 adapterID = -1;

#ifdef NDEBUG
				bool enableDebugLayers = false;
#else
				bool enableDebugLayers = true;
#endif
			};

			OutputMode outputMode = OutputMode::Window;
			WindowConfig output;
			EngineConfig engine;
			RendererConfig render;

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
				if (output.title.empty())
					return appName;
				else
					return output.title;
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
		virtual void OnBeginFrame() {}

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
	};
}
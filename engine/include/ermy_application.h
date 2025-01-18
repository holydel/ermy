#pragma once

namespace ermy
{
	class Application
	{
	private:

	public:
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
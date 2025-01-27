#include "swapchain_interface.h"
#include "ermy_log.h"

swapchain::Config swapchain::gConfig;

void swapchain::Configure(const ermy::Application::StaticConfig& appConfig)
{
	ERMY_LOG("SWAPCHAIN Configure");

	gConfig.useDisplay = appConfig.outputMode == ermy::Application::StaticConfig::OutputMode::Display;
	gConfig.resizable = appConfig.window.mode == ermy::Application::StaticConfig::WindowConfig::WindowMode::Windowed;
	gConfig.needExclusiveFullscreen = appConfig.window.mode == ermy::Application::StaticConfig::WindowConfig::WindowMode::ExclusiveFullscreen; 	
}
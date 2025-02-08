#include "swapchain_interface.h"
#include "ermy_log.h"

swapchain::Config swapchain::gSwapchainConfig;

void swapchain::Configure(const ermy::Application::StaticConfig& appConfig)
{
	ERMY_LOG("SWAPCHAIN Configure");

	gSwapchainConfig.useDisplay = appConfig.outputMode == ermy::Application::StaticConfig::OutputMode::Display;
	gSwapchainConfig.resizable = appConfig.window.mode == ermy::Application::StaticConfig::WindowConfig::WindowMode::Windowed;
	gSwapchainConfig.needExclusiveFullscreen = appConfig.window.mode == ermy::Application::StaticConfig::WindowConfig::WindowMode::ExclusiveFullscreen; 	

	gSwapchainConfig.swapchainConfig = appConfig.swapchain;
}
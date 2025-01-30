#pragma once
#include "ermy_api.h"
#include "ermy_application.h"

namespace swapchain
{
    struct Config
    {
        bool useDisplay : 1 = false;
		bool resizable : 1 = false;
        bool needExclusiveFullscreen : 1 = false;

        ermy::Application::StaticConfig::SwapchainConfig swapchainConfig = {};
    };

    extern Config gConfig;

    void Configure(const ermy::Application::StaticConfig& appConfig);

    void Initialize();
    void Shutdown();

    void Process();

    void AcquireNextImage();
    void Present();

    int GetNumFrames();

    bool ReInitIfNeeded();
}
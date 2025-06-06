﻿#include "application.h"
#include <cassert>
#include "os/os.h"
#include "logger.h"

#include "rendering/rendering.h"
#include "sound/sound_interface.h"
#include "xr/xr_interface.h"

ermy::Application *gApplication = nullptr;

ermy::Application &GetApplication()
{
	return *gApplication;
}

ermy::Application::Application()
{
	// only one application per executable may be used
	assert(gApplication == nullptr);
	gApplication = this;
}

const ermy::Application* ermy::Application::GetApplication()
{
	return gApplication;
}

ermy::Application::~Application()
{
	gApplication = nullptr;
}

void ErmyApplicationStart()
{
	assert(gApplication != nullptr);
	gApplication->OnConfigure();

	const auto& config = gApplication->staticConfig;
	
	// initialize engine
	loggerImpl::Initialize();
	ERMY_LOG(u8"start initialize ermy engine for application: %s", config.appName.c_str()); //

	if(config.engine.EnableXR)
		xr_interface::Initialize();

	if(config.HasOutputWindow())
		os::CreateNativeWindow();

	sound_interface::Initialize();
	rendering::Initialize();

	if (config.engine.EnableXR)
		xr_interface::CreateSession();
	
	gApplication->OnInitialization();

	// initialize engine built-in data

	gApplication->OnLoad();
}

bool ErmyApplicationStep()
{
	os::Update();

	gApplication->OnUpdate();

	rendering::BeginFrame();
	//gApplication->OnBeginFrame();

	rendering::Process();
	sound_interface::Process();
	//gApplication->OnEndFrame();

	rendering::EndFrame();
	
	
	return GetApplication().IsRunning();
}

void ErmyApplicationShutdown()
{
	gApplication->OnUnLoad();

	gApplication->OnShutdown();

	rendering::Shutdown();
	sound_interface::Shutdown();

	xr_interface::Shutdown();
	loggerImpl::Shutdown();
}

void ErmyApplicationRun() //some systems (emscriten, macos) have dedicated step functions, so this is just a wrapper
{
	ErmyApplicationStart();

	while (true)
	{
		if(!ErmyApplicationStep())
			break;
	}

	ErmyApplicationShutdown();
}
#include "application.h"
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

	// initialize engine
	loggerImpl::Initialize();
	ERMY_LOG("start initialize ermy engine for application: %s", gApplication->staticConfig.appName.c_str()); //

	xr_interface::Initialize();

	os::CreateNativeWindow();
	sound_interface::Initialize();
	rendering::Initialize();
	gApplication->OnInitialization();

	// initialize engine built-in data

	gApplication->OnLoad();
}

bool ErmyApplicationStep()
{
	os::Update();

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
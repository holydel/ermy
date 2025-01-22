#include "application.h"
#include <cassert>
#include "os/os.h"
#include "logger.h"

#include "rendering/rendering_interface.h"
#include "rendering/swapchain_interface.h"

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

	os::CreateNativeWindow();
	rendering::Initialize();
	swapchain::Initialize();
	gApplication->OnInitialization();

	// initialize engine built-in data

	gApplication->OnLoad();
}

bool ErmyApplicationStep()
{
	gApplication->OnBeginFrame();

	rendering::Process();
	swapchain::Process();

	gApplication->OnEndFrame();

	os::Update();
	return true;
}

void ErmyApplicationShutdown()
{
	gApplication->OnUnLoad();

	gApplication->OnShutdown();

	swapchain::Shutdown();
	rendering::Shutdown();
	loggerImpl::Shutdown();
}

void ErmyApplicationRun()
{
	ErmyApplicationStart();

	while (true)
	{
		if(!ErmyApplicationStep())
			break;
	}

	ErmyApplicationShutdown();
}
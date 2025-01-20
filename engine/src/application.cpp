#include "application.h"
#include <cassert>
#include "os/ermy_os.h"
#include "logger.h"

#include "rendering/rendering_interface.h"

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
	ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Debug, "start initialize ermy engine for application: %s", gApplication->staticConfig.appName.c_str()); //

	os::CreateNativeWindow();
	rendering::Initialize();

	gApplication->OnInitialization();

	// initialize engine built-in data

	gApplication->OnLoad();
}

bool ErmyApplicationStep()
{
	gApplication->OnBeginFrame();

	rendering::Process();
	
	gApplication->OnEndFrame();

	os::Update();
	return true;
}

void ErmyApplicationShutdown()
{
	gApplication->OnUnLoad();

	gApplication->OnShutdown();

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
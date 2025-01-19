#include "application.h"
#include <cassert>
#include "os/ermy_os_utils.h"
#include "logger.h"

ermy::Application* gApplication = nullptr;

ermy::Application::Application()
{
	//only one application per executable may be used
	assert(gApplication == nullptr);
	gApplication = this;
}

ermy::Application::~Application()
{
	gApplication = nullptr;
}

void ErmyApplicationRun()
{
	assert(gApplication != nullptr);
	gApplication->OnConfigure();

	//initialize engine
	loggerImpl::Initialize();



	ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Debug, "start initialize ermy engine for application: %s", gApplication->staticConfig.name.c_str()); //
	gApplication->OnInitialization();

	//initialize engine built-in data

	gApplication->OnLoad();

	while (true)
	{
		gApplication->OnBeginFrame();
		gApplication->OnEndFrame();

		os::Sleep(1);
	}
	
	gApplication->OnUnLoad();

	gApplication->OnShutdown();

	loggerImpl::Shutdown();
}
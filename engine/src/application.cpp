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

	gApplication->OnInitialization();

	//initialize engine built-in data

	gApplication->OnLoad();

	while (true)
	{
		gApplication->OnBeginFrame();
		gApplication->OnEndFrame();

		os::sleep(1);
	}
	
	gApplication->OnUnLoad();

	gApplication->OnShutdown();

	loggerImpl::Shutdown();
}
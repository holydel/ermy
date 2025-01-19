#include "application.h"
#include <cassert>
#include "os/ermy_os.h"
#include "logger.h"

ermy::Application* gApplication = nullptr;

ermy::Application& GetApplication()
{
	return *gApplication;
}

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
	ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Debug, "start initialize ermy engine for application: %s", gApplication->staticConfig.appName.c_str()); //
	
	os::CreateNativeWindow();

	gApplication->OnInitialization();

	//initialize engine built-in data

	gApplication->OnLoad();

	while (true)
	{
		gApplication->OnBeginFrame();
		gApplication->OnEndFrame();

		os::Update();		
	}
	
	gApplication->OnUnLoad();

	gApplication->OnShutdown();

	loggerImpl::Shutdown();
}
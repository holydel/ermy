#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
using namespace ermy;

class TestBedApplication : public ermy::Application
{
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "TestBed";

		staticConfig.render.adapterID = 1;
	}

	void OnInitialization() override;

	void OnEndFrame() override;
};

static TestBedApplication myTestBedApp;

void TestBedApplication::OnInitialization()
{
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose, "TESTBED", "Test Verbose Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Debug, "TESTBED", "Test Debug Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Warning, "TESTBED", "Test Warning Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Error, "TESTBED", "Test Error Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Fatal, "TESTBED", "Test Fatal Message");
}

void TestBedApplication::OnEndFrame()
{
	static float a = 0.0f;
	a += 0.01f;

	float r = sin(a * 1.5f) * 0.5f + 0.5f;
	float g = cos(a * 2.5f) * 0.5f + 0.5f;
	float b = (sin(a * 3.5f) + cos(a * 0.5f)) * 0.25 + 0.5f;
	canvas::SetClearColor(r,g,b,1.0f);
	
}
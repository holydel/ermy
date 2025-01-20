#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"

class TestBedApplication : public ermy::Application
{
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "TestBed";
	}

	void OnInitialization() override;
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
#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
#include <ermy_input.h>

using namespace ermy;

class TestBedApplication : public ermy::Application
{
	rendering::PSOID testTrianglePSO;
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "TestBed";

		staticConfig.render.adapterID = 0;
		staticConfig.render.vkConfig.useDynamicRendering = true;
	}

	void OnInitialization() override;
	void OnLoad() override;
	void OnBeginFrame(rendering::CommandList& finalCL) override;
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

void TestBedApplication::OnLoad()
{
	rendering::PSODesc desc;
	desc.shaders.push_back(shader_internal::testTriangleVS());
	desc.shaders.push_back(shader_internal::testTriangleFS());
	desc.numRootConstants = 0;

	testTrianglePSO = rendering::CreatePSO(desc);
}

void TestBedApplication::OnBeginFrame(rendering::CommandList& finalCL)
{
	finalCL.SetPSO(testTrianglePSO);
	finalCL.Draw(3);

	
	auto mpos = ermy::input::mouse::GetCurrentPosition();

	static float a = 0.0f;
	a += 0.1f;

	canvas::DrawDedicatedSprite(mpos.x, mpos.y, 80, 10, a);
}
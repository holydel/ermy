#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
#include <ermy_input.h>
#include <ermy_pak.h>
#include <ermy_scene.h>
#include <ermy_geometry.h>
#include <random>

#include <Windows.h>

using namespace ermy;

class TestBedApplication : public ermy::Application
{
	rendering::PSOID testTrianglePSO;

	scene::SceneID sceneId;
	scene::GeometryID cubeGeom;

	std::vector<scene::EntityID> allCubes;
	std::vector<glm::quat> allCubesDeltaRotation;
	const int numCubes = 100;

	glm::quat currentCameraOrient = glm::identity<glm::quat>();
	bool isRunningValue = true;

	ermy::Font* atlasFont = nullptr;
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = u8"TestBed";

		staticConfig.render.enableDebugLayers = false;
		staticConfig.render.adapterID = 0;

		staticConfig.window.supportTransparent = false;
		staticConfig.window.mode = ermy::Application::StaticConfig::WindowConfig::WindowMode::Windowed;

		staticConfig.swapchain.depthMode = ermy::Application::StaticConfig::SwapchainConfig::DepthMode::Depth16;
		staticConfig.swapchain.msaaMode = ermy::Application::StaticConfig::SwapchainConfig::MSAAMode::Samples4;
		staticConfig.swapchain.vsync = ermy::Application::StaticConfig::SwapchainConfig::VSyncMode::AlwaysVSync;

		staticConfig.swapchain.tripleBuffering = false;
		//staticConfig.render.vkConfig.useDynamicRendering = false;
	}

	void OnInitialization() override;
	void OnLoad() override;
	void OnUpdate() override;
	void OnBeginFinalPass(rendering::CommandList& finalCL) override;
	void OnEndFrame() override;

	bool IsRunning() override
	{
		return isRunningValue;
	}
};

static TestBedApplication myTestBedApp;

void TestBedApplication::OnInitialization()
{
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose, u8"TESTBED", u8"Test Verbose Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Debug, u8"TESTBED", u8"Test Debug Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Warning, u8"TESTBED", u8"Test Warning Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Error, u8"TESTBED", u8"Test Error Message世界こんにちは");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Fatal, u8"TESTBED", u8"Test Fatal Message©\u00df\u6c34\U0001d10b");
}

void TestBedApplication::OnUpdate()
{
	//currentCameraOrient *= glm::quat(glm::vec3(0.01f, 0.000f, 0.000f));
	auto mpos = input::mouse::GetCurrentPosition();
	auto mdelta = input::mouse::GetDeltaPosition();

	//currentCameraOrient = glm::quat(glm::vec3((float)mpos.y * 0.01f, (float)mpos.x * 0.01f, 0.0f));
	currentCameraOrient *= glm::quat(glm::vec3(mdelta.y * -64.0f, mdelta.x * -64.0f, 0.0f));

	if (input::gamepad::IsConnected(0))
	{
		auto& s = input::gamepad::GetState(0);
		currentCameraOrient *= glm::quat(glm::vec3(s.leftStick.x * 0.01f, s.leftStick.y * 0.01f, 0.0f));
	}
	

	scene::SetCameraOrientation(currentCameraOrient);

	for (int i = 0; i < numCubes; ++i)
	{
		scene::GetTransform(scene::EntityID(i)).orientation = allCubesDeltaRotation[i] * scene::GetTransform(scene::EntityID(i)).orientation;
	}

	if(input::keyboard::IsKeyPressed(input::keyboard::KeyCode::Escape))
	{
		isRunningValue = false;
	}
}

void TestBedApplication::OnEndFrame()
{
	static float a = 0.0f;
	a += 0.01f;

	float r = sin(a * 1.5f) * 0.5f + 0.5f;
	float g = cos(a * 2.5f) * 0.5f + 0.5f;
	float b = (sin(a * 3.5f) + cos(a * 0.5f)) * 0.25 + 0.5f;
	canvas::SetClearColor(r,g,b,0.0f);
	
}

glm::quat randomUnitQuaternionAxisAngle()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	// Random angle [0, 2pi]
	float angle = dis(gen) * 2.0f * glm::pi<float>();

	// Random unit vector (Marsaglia’s method)
	float x, y, z;
	float s, t;
	do {
		x = dis(gen) * 2.0f - 1.0f;
		y = dis(gen) * 2.0f - 1.0f;
		s = x * x + y * y;
	} while (s >= 1.0f);
	t = std::sqrt(1.0f - s);
	z = (dis(gen) < 0.5f) ? -t : t;
	glm::vec3 axis(x, y, z);
	axis = glm::normalize(axis);

	return glm::angleAxis(angle, axis);
}

void TestBedApplication::OnLoad()
{
	rendering::PSODesc desc;
	desc.SetShaderStageInternal(shader_internal::testTriangleVS());
	desc.SetShaderStageInternal(shader_internal::testTriangleFS());
	desc.debugName = "TestTriangle";
	testTrianglePSO = rendering::CreatePSO(desc);

#ifdef _WIN32
	ermy::pak::MountPak(u8"D:\\Projects\\ermy\\eproj_template\\paks\\0.epak");
#else
	ermy::pak::MountPak(u8"/sdcard/Android/data/com.hexcelltechvr.ermy.testbed/files/0.epak");
#endif

	sceneId = scene::Create();
	scene::SetSkyBoxTexture(rendering::TextureID(1));

	cubeGeom = scene::LoadSubmesh(geometry::CreateCubeGeometry(1.0f));
	allCubes.resize(numCubes);
	allCubesDeltaRotation.resize(numCubes);
	glm::quat identityQuat = glm::identity<glm::quat>();

	for (int i = 0; i < numCubes; ++i)
	{
		scene::Transform transform;
		transform.position_uniform_scale.x = (((float)(rand() % RAND_MAX) / (float)RAND_MAX) - 0.5f) * 200.0f;
		transform.position_uniform_scale.y = (((float)(rand() % RAND_MAX) / (float)RAND_MAX) - 0.5f) * 200.0f;
		transform.position_uniform_scale.z = (((float)(rand() % RAND_MAX) / (float)RAND_MAX) - 0.5f) * 200.0f;

		transform.orientation = randomUnitQuaternionAxisAngle();
		allCubesDeltaRotation[i] = glm::slerp(identityQuat, randomUnitQuaternionAxisAngle(), 0.01f);
		allCubes[i] = scene::AddEntity(cubeGeom, transform);
	}

	atlasFont = ermy::Font::CreateFromFile(u8"D:\\Projects\\ermy\\eproj_template\\assets\\UnGraphicBold.ttf");
}

void TestBedApplication::OnBeginFinalPass(rendering::CommandList& finalCL)
{
	//finalCL.SetPSO(testTrianglePSO);
	//finalCL.Draw(3);
	
	auto mpos = ermy::input::mouse::GetCurrentPosition();

	static float a = 0.0f;
	a += 0.1f;

	canvas::DrawDedicatedSprite(rendering::TextureID(3), 300, 300, 200, 200);

	canvas::DrawDedicatedSprite(mpos.x, mpos.y, 120, 20, a, 0xAA7722FFu);

}
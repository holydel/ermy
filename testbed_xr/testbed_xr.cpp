#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"
#include <ermy_input.h>
#include <ermy_pak.h>
#include <ermy_scene.h>
#include <ermy_geometry.h>
#include <random>

using namespace ermy;

class TestBedXRApplication : public ermy::Application
{
	rendering::PSOID testTrianglePSO;

	scene::SceneID sceneId;
	scene::GeometryID cubeGeom;

	std::vector<scene::EntityID> allCubes;
	std::vector<glm::quat> allCubesDeltaRotation;
	const int numCubes = 1000;

	glm::quat currentCameraOrient = glm::identity<glm::quat>();
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "TestBedXR";

		staticConfig.render.enableDebugLayers = true;
		staticConfig.render.adapterID = 0;

		staticConfig.swapchain.depthMode = ermy::Application::StaticConfig::SwapchainConfig::DepthMode::Depth32F;
		staticConfig.swapchain.msaaMode = ermy::Application::StaticConfig::SwapchainConfig::MSAAMode::Samples4;

		//staticConfig.render.vkConfig.useDynamicRendering = false;
	}

	void OnInitialization() override;
	void OnLoad() override;
	void OnUpdate() override;
	void OnBeginFinalPass(rendering::CommandList& finalCL) override;
	void OnEndFrame() override;
};

static TestBedXRApplication myTestBedApp;

void TestBedXRApplication::OnInitialization()
{
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose, "TESTBED", "Test Verbose Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Debug, "TESTBED", "Test Debug Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Warning, "TESTBED", "Test Warning Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Error, "TESTBED", "Test Error Message");
	ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Fatal, "TESTBED", "Test Fatal Message");
}

void TestBedXRApplication::OnUpdate()
{
	//currentCameraOrient *= glm::quat(glm::vec3(0.01f, 0.000f, 0.000f));
	auto mpos = input::mouse::GetCurrentPosition();

	currentCameraOrient = glm::quat(glm::vec3((float)mpos.y * 0.01f, (float)mpos.x * 0.01f, 0.0f));
	scene::SetCameraOrientation(currentCameraOrient);

	for (int i = 0; i < numCubes; ++i)
	{
		scene::GetTransform(scene::EntityID(i)).orientation = allCubesDeltaRotation[i] * scene::GetTransform(scene::EntityID(i)).orientation;
	}
}

void TestBedXRApplication::OnEndFrame()
{
	static float a = 0.0f;
	a += 0.01f;

	float r = sin(a * 1.5f) * 0.5f + 0.5f;
	float g = cos(a * 2.5f) * 0.5f + 0.5f;
	float b = (sin(a * 3.5f) + cos(a * 0.5f)) * 0.25 + 0.5f;
	canvas::SetClearColor(r, g, b, 0.0f);

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

void TestBedXRApplication::OnLoad()
{
	rendering::PSODesc desc;
	desc.SetShaderStageInternal(shader_internal::testTriangleVS());
	desc.SetShaderStageInternal(shader_internal::testTriangleFS());
	desc.debugName = "TestTriangle";
	testTrianglePSO = rendering::CreatePSO(desc);

#ifdef _WIN32
	ermy::pak::MountPak("D:\\Projects\\ermy\\eproj_template\\paks\\0.epak");
#else
	ermy::pak::MountPak("/sdcard/Android/data/com.hexcelltechvr.ermy.testbed/files/0.epak");
#endif

	scene::SceneDesc sdesc = {};
	sdesc.isXRScene = true;

	sceneId = scene::Create(sdesc);
	scene::SetSkyBoxTexture(rendering::TextureID(1));

	cubeGeom = scene::LoadSubmesh(geometry::CreateCubeGeometry(1.0f));
	allCubes.resize(numCubes);
	allCubesDeltaRotation.resize(numCubes);
	glm::quat identityQuat = glm::identity<glm::quat>();

	for (int i = 0; i < numCubes; ++i)
	{
		scene::Transform transform;
		transform.position_uniform_scale.x = (((float)(rand() % RAND_MAX) / (float)RAND_MAX) - 0.5f) * 100.0f;
		transform.position_uniform_scale.y = (((float)(rand() % RAND_MAX) / (float)RAND_MAX) - 0.5f) * 100.0f;
		transform.position_uniform_scale.z = (((float)(rand() % RAND_MAX) / (float)RAND_MAX) - 0.5f) * 100.0f;

		transform.orientation = randomUnitQuaternionAxisAngle();
		allCubesDeltaRotation[i] = glm::slerp(identityQuat, randomUnitQuaternionAxisAngle(), 0.01f);
		allCubes[i] = scene::AddEntity(cubeGeom, transform);
	}
}

void TestBedXRApplication::OnBeginFinalPass(rendering::CommandList& finalCL)
{
	//finalCL.SetPSO(testTrianglePSO);
	//finalCL.Draw(3);

	auto mpos = ermy::input::mouse::GetCurrentPosition();

	static float a = 0.0f;
	a += 0.1f;

	canvas::DrawDedicatedSprite(rendering::TextureID(3), 300, 300, 200, 200);

	canvas::DrawDedicatedSprite(mpos.x, mpos.y, 120, 20, a, 0xAA7722FFu);

}
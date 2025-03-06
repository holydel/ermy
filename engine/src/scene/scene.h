#pragma once
#include <ermy_scene.h>
#include <vector>
#include <ermy_commandlist.h>

struct SceneMeta
{
	ermy::scene::SceneDesc initialDesc;

	ermy::rendering::BufferID staticVertices;
	ermy::rendering::BufferID staticIndices;
	ermy::rendering::BufferID skinnedVertices;
	ermy::rendering::BufferID skinnedIndices;

	ermy::rendering::PSOID skyBoxPass;
	ermy::rendering::PSOID internalColoredMeshesPass;
	
	ermy::rendering::PSOID skyBoxPassXR;
	ermy::rendering::PSOID internalColoredMeshesPassXR;

	bool isLoaded = false;
	bool isActive = false;

	int currentVertexOffset = 0;
	int currentIndexOffset = 0;

	ermy::rendering::TextureID skyboxTex;

	glm::vec3 cameraPosition = glm::vec3(0,0,0);
	glm::quat cameraOrientation = glm::identity<glm::quat>();
	float cameraVerticalFOV = 1.25f;
	float cameraNearZ = 0.1f;
	float cameraFarZ = 1000.0f;
	float cameraAspect = 1.0f;

	glm::mat4 projMatrix = glm::identity<glm::mat4>();
	glm::mat4 viewMatrix = glm::identity<glm::mat4>();

	glm::vec3 cameraViewDir = glm::vec3(0.0f, 0.0f, -1.0f);
	ermy::u64 sceneDescriptorSet;

	std::vector<ermy::rendering::SubMesh> allGeometries;

	std::vector<ermy::scene::GeometryID> entityGeometries;
	std::vector<ermy::scene::Transform> entityTransformsWorld;
};

extern std::vector<SceneMeta> gAllScenes3D;

namespace scene_internal
{
	void UpdateUniforms();
	void Render(ermy::rendering::CommandList& cl, bool isXRPass);
}
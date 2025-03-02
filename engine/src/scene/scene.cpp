#include "scene.h"
#include <glm/glm.hpp>
#include "../rendering/rendering.h"
#include <ermy_shader_internal.h>
#include "../rendering/framegraph.h"

using namespace ermy;
using namespace ermy::scene;

std::vector<SceneMeta> gAllScenes3D;
SceneID gCurrentSceneID;

void ermy::scene::LoadFromPak(SceneID sceneID)
{

}

void ermy::scene::Unload(SceneID sceneID)
{

}

SceneID ermy::scene::Create(const SceneDesc& desc)
{
	auto& scene = gAllScenes3D.emplace_back();
	SceneID result = static_cast<SceneID>(gAllScenes3D.size() - 1);

	scene.initialDesc = desc;
	scene.isLoaded = true;

	scene.staticVertices = rendering::CreateDedicatedBuffer(rendering::BufferDesc
		{ .size = desc.totalStaticVertices,
		.usage = rendering::BufferUsage::Vertex,
		.debugName = "Scene Static Vertices Buffer",
		.persistentMapped = true});

	scene.staticIndices = rendering::CreateDedicatedBuffer(rendering::BufferDesc
		{ .size = desc.totalStaticIndices,
		.usage = rendering::BufferUsage::Index,
		.debugName = "Scene Static Index Buffer",
		.persistentMapped = true });

	scene.skinnedVertices = rendering::CreateDedicatedBuffer(rendering::BufferDesc
		{ .size = desc.totalSkinnedVertices,
		.usage = rendering::BufferUsage::Vertex,
		.debugName = "Scene Skinned Vertices Buffer",
		.persistentMapped = true });

	scene.skinnedIndices = rendering::CreateDedicatedBuffer(rendering::BufferDesc
		{ .size = desc.totalSkinnedIndices,
		.usage = rendering::BufferUsage::Index,
		.debugName = "Scene Skinned Index Buffer",
		.persistentMapped = true });



	rendering::TextureDesc dummySkyboxTexDesc;
	dummySkyboxTexDesc.debugName = "dummySkyboxTex";
	dummySkyboxTexDesc.width = 1;
	dummySkyboxTexDesc.height = 1;
	dummySkyboxTexDesc.numLayers = 6;
	dummySkyboxTexDesc.isCubemap = 1;
	dummySkyboxTexDesc.dataSize = 6 * sizeof(glm::vec4);

	glm::vec4 dummyData[6] = {
		{0.3,0.31,0.65,1.0f}, //+X
		{0.2,0.31,0.65,1.0f}, //-X
		{0.4,0.6,1.0,1.0f}, //+Y
		{0.1,0.3,0.15,1.0f}, //-Y
		{0.15,0.31,0.75,1.0f}, //+Z
		{0.15,0.31,0.55,1.0f}, //-Z
	};
	dummySkyboxTexDesc.pixelsData = dummyData;
	dummySkyboxTexDesc.texelFormat = ermy::rendering::Format::RGBA32F;
	/*Index 0: +X(Positive X) - Right face
		Index 1 : -X(Negative X) - Left face
		Index 2 : +Y(Positive Y) - Top face
		Index 3 : -Y(Negative Y) - Bottom face
		Index 4 : +Z(Positive Z) - Front face
		Index 5 : -Z(Negative Z) - Back face*/

	scene.skyboxTex = rendering::CreateDedicatedTexture(dummySkyboxTexDesc);


	rendering::PSODesc skyBox;
	skyBox.debugName = "SkyBox PSO";
	skyBox.SetShaderStage(ermy::shader_internal::skyboxVS());
	skyBox.SetShaderStage(ermy::shader_internal::skyboxFS());
	skyBox.domain = rendering::PSODomain::Scene;

	scene.skyBoxPass = rendering::CreatePSO(skyBox);

	ermy::rendering::DescriptorSetDesc dsDesc;
	dsDesc.AddBindingUniformBuffer(gFrameConstants);
	dsDesc.AddBindingTextureCube(scene.skyboxTex);
	scene.sceneDescriptorSet = ermy::rendering::CreateDescriptorSet(rendering::PSODomain::Scene, dsDesc);

	SetActive(result, true);

	return result;
}

void ermy::scene::SetActive(SceneID sceneId, bool status)
{
	auto& scene = gAllScenes3D[sceneId.handle];
	scene.isActive = status;

	if (status)
	{
		SetCurrent(sceneId);
	}
}

void ermy::scene::SetCurrent(SceneID sceneId)
{
	gCurrentSceneID = sceneId;
}

ermy::rendering::SubMesh ermy::scene::LoadSubmesh(int numVertices, int numIndices, rendering::StaticVertexDedicated* vertices, u16* indices)
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];
	
	rendering::SubMesh result;
	result.indexCount = numIndices;
	result.vertexCount = numVertices;
	result.indexOffset = scene.currentIndexOffset;
	result.vertexOffset = scene.currentVertexOffset;

	rendering::UpdateBufferData(scene.staticVertices, result.vertexOffset * sizeof(rendering::StaticVertexDedicated), vertices, result.vertexCount * sizeof(rendering::StaticVertexDedicated));
	rendering::UpdateBufferData(scene.staticIndices, result.indexOffset * sizeof(u16), indices, result.indexCount * sizeof(u16));

	scene.currentVertexOffset += numVertices;
	scene.currentIndexOffset += numIndices;


	return result;
}

void ermy::scene::SetCameraPosition(const glm::vec3& position)
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];
	scene.cameraPosition = position;
}

void ermy::scene::SetCameraOrientation(const glm::quat& orientation)
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];
	scene.cameraOrientation = orientation;
}

void ermy::scene::SetCameraVerticalFOV(float fovRadians)
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];
	scene.cameraVerticalFOV = fovRadians;
}

void ermy::scene::SetSkyBoxTexture(rendering::TextureID texture)
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];

	ermy::rendering::DescriptorSetDesc::Binding b = {
		.bindingSlot = 1,
		.uniformType = ermy::rendering::ShaderUniformType::TextureCube
	};

	b.textureInfo.texture = texture;

	ermy::rendering::UpdateDescriptorSet(scene.sceneDescriptorSet, b);
}

void scene_internal::UpdateUniforms()
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];
	scene.projMatrix = glm::perspectiveRH_ZO(scene.cameraVerticalFOV, scene.cameraAspect, scene.cameraNearZ, scene.cameraFarZ);

	glm::mat3 rotation = glm::mat3_cast(scene.cameraOrientation);
	glm::mat4 rotationMatrix = glm::mat4(rotation);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -scene.cameraPosition);
	scene.viewMatrix = rotationMatrix * translationMatrix;

	// Default forward direction in camera space (negative Z for right-handed system)
	glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);

	// Rotate the default forward vector by the camera orientation
	scene.cameraViewDir = scene.cameraOrientation * defaultForward;

	glm::mat4 viewProjMat = scene.viewMatrix * scene.projMatrix;
	glm::mat4 viewProjMatInv = glm::inverse(viewProjMat);

	glm::mat4 projMatInv = glm::inverse(scene.projMatrix);
	glm::mat4 viewMatInv = glm::inverse(scene.viewMatrix);

	for (int i = 0; i < 2; ++i)
	{
		gErmyFrameConstants.CameraWorldPos[i] = glm::vec4(scene.cameraPosition, 1.0);
		gErmyFrameConstants.CameraViewDir[i] = glm::vec4(scene.cameraViewDir, 1.0);

		gErmyFrameConstants.ViewMatrix[i] = scene.viewMatrix;
		gErmyFrameConstants.ProjMatrix[i] = scene.projMatrix;
		gErmyFrameConstants.ViewMatrixInv[i] = viewMatInv;
		gErmyFrameConstants.ProjMatrixInv[i] = projMatInv;

		gErmyFrameConstants.ViewProjMatrix[i] = viewProjMat;
		gErmyFrameConstants.ViewProjMatrixInv[i] = viewProjMatInv;
	}

}

void scene_internal::Render(ermy::rendering::CommandList& cl)
{
	auto& scene = gAllScenes3D[gCurrentSceneID.handle];

	cl.SetPSO(scene.skyBoxPass);
	cl.SetDescriptorSet(0, scene.sceneDescriptorSet);
	cl.Draw(3);
}
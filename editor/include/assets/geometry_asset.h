#pragma once

#include <assets/asset.h>
#include <ermy_rendering.h>
#include <ermy_commandlist.h>
#include <glm/gtc/quaternion.hpp>

class GeometryAsset : public AssetData
{
	bool isStaticPreview = false;	
	float Zoom = 1.0f;
	glm::quat rotationQuat = glm::identity<glm::quat>(); // Quaternion for arcball rotation
	glm::quat lastRotationQuat = glm::identity<glm::quat>(); // Quaternion for arcball rotation
	glm::vec2 lastMousePos = glm::vec2(0,0); // Last mouse position for arcball dragging

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.5f);
	glm::vec3 oldCamPosition = cameraPos;
	
	void RecalculateBoundingSphere();
	int currentMode = 0;
public:
	AssetDataType GetDataType() override { return AssetDataType::Geometry; }
	glm::vec4 boundingSphere =  glm::vec4(0); //xyz - center, w - radius
	std::vector<ermy::u16> allIndices;
	std::vector<ermy::rendering::StaticVertexDedicated> allVertices;
	std::vector<ermy::rendering::SubMesh> subMeshes;

	ermy::u32 numVertices;
	ermy::u32 numIndices;
	ermy::rendering::DedicatedMesh previewMesh;

	GeometryAsset();
	virtual ~GeometryAsset();

	void DrawPreview() override;

	void RegenerateLivePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY, int button) override;
	void MouseUp(int button) override;
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button) override;
	void ResetView() override;

	void Save(pugi::xml_node& node) override;
	void Load(pugi::xml_node& node) override;

	std::vector<ermy::u8> GetStaticPreviewData() override;

	void LoadFromCachedRaw(std::ifstream& file, const std::filesystem::path& path) override;
	void SaveToCachedRaw(std::ofstream& file) override;
};
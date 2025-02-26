#pragma once

#include <assets/asset.h>
#include <ermy_rendering.h>
#include <ermy_commandlist.h>

class GeometryAsset : public AssetData
{
	bool isStaticPreview = false;	
	float Zoom = 1.0f;
	float Pitch = 0.0f;
	float Yaw = 0.0f;
	float OldPitch = 0.0f;
	float OldYaw = 0.0f;
	bool isPreviewDragging = false;
	void RecalculateBoundingSphere();
	int currentMode = 0;
public:
	AssetDataType GetDataType() override { return AssetDataType::Geometry; }
	glm::vec4 boundingSphere =  glm::vec4(0); //xyz - center, w - radius
	std::vector<ermy::u16> allIndices;
	std::vector<ermy::rendering::StaticVertexDedicated> allVertices;
	std::vector<ermy::rendering::DedicatedMesh::SubMesh> subMeshes;

	ermy::u32 numVertices;
	ermy::u32 numIndices;
	ermy::rendering::DedicatedMesh previewMesh;

	GeometryAsset();
	virtual ~GeometryAsset();

	void DrawPreview() override;

	void RegeneratePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY) override;
	void MouseUp() override;
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY) override;
	void ResetView() override;
};
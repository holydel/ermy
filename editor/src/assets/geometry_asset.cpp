#include <assets/geometry_asset.h>
#include <ermy_log.h>
#include <preview_renderer.h>
#include <editor_shader_internal.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Assimp::Importer* gAssetImporter = nullptr;
using namespace ermy;
using namespace ermy::rendering;

class GeometryRenderPreview
{
	GeometryRenderPreview();
	~GeometryRenderPreview();

	rendering::PSOID fullscreenEmpty;
	rendering::PSOID renderMeshUV;
public:
	static GeometryRenderPreview& Instance()
	{
		static GeometryRenderPreview instance;
		return instance;
	}

	void BindCheckerPSO(rendering::CommandList& cl)
	{
		cl.SetPSO(fullscreenEmpty);
	}

	void BindPSO(rendering::CommandList& cl, bool isStatic = false)
	{
		cl.SetPSO(renderMeshUV);
	}
};

GeometryRenderPreview::GeometryRenderPreview()
{
	auto RTT = PreviewRenderer::Instance().GetRTT();
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	//live preview PSO
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSEmpty());
		desc.specificRenderPass = RTT;
		fullscreenEmpty = rendering::CreatePSO(desc);
	}	

	//render mesh PSO
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::dedicatedStaticMeshVS());
		desc.SetShaderStage(ermy::shader_editor::dedicatedStaticMeshFS());
		desc.specificRenderPass = RTT;
		desc.rootConstantRanges[(int)ermy::ShaderStage::Vertex].size = 64;
		desc.rootConstantRanges[(int)ermy::ShaderStage::Fragment].size = 68;

		desc.vertexAttributes.push_back({ rendering::Format::RGB32F }); //float x, y, z;
		desc.vertexAttributes.push_back({ rendering::Format::RGB32F }); //float nx, ny, nz;
		desc.vertexAttributes.push_back({ rendering::Format::RGB32F }); //float tx, ty, tz;
		desc.vertexAttributes.push_back({ rendering::Format::RGB32F }); //float bx, by, bz;
		desc.vertexAttributes.push_back({ rendering::Format::RG32F }); //float u0, v0;
		desc.vertexAttributes.push_back({ rendering::Format::RG32F }); //float u1, v1;
		desc.vertexAttributes.push_back({ rendering::Format::RGBA32F }); //float r, g, b, a;

		desc.writeDepth = true;
		desc.testDepth = true;
		renderMeshUV = rendering::CreatePSO(desc);
	}
}

GeometryRenderPreview::~GeometryRenderPreview()
{

}

void GeometryAsset::RecalculateBoundingSphere()
{
	//BoundingSphere CalculateBoundingSphere(const std::vector<glm::vec3>&vertices) {

		if (allVertices.empty()) {
			return;
		}

		// Step 1: Find the minimum and maximum points along each axis
		
		glm::vec3 minX = { allVertices[0].x,allVertices[0].y,allVertices[0].z };
		
		glm::vec3 maxX = minX;
		glm::vec3 minY = minX, maxY = minX;
		glm::vec3 minZ = minX, maxZ = minX;

		for (const auto& vertex : allVertices) {
			
			glm::vec3 vpos = { vertex.x,vertex.y,vertex.z };

			if (vertex.x < minX.x) minX = vpos;
			if (vertex.x > maxX.x) maxX = vpos;
			if (vertex.y < minY.y) minY = vpos;
			if (vertex.y > maxY.y) maxY = vpos;
			if (vertex.z < minZ.z) minZ = vpos;
			if (vertex.z > maxZ.z) maxZ = vpos;
		}

		// Step 2: Find the pair of points farthest apart
		float distX = glm::length(maxX - minX);
		float distY = glm::length(maxY - minY);
		float distZ = glm::length(maxZ - minZ);

		glm::vec3 diamEnd1, diamEnd2;
		if (distX > distY && distX > distZ) {
			diamEnd1 = minX;
			diamEnd2 = maxX;
		}
		else if (distY > distZ) {
			diamEnd1 = minY;
			diamEnd2 = maxY;
		}
		else {
			diamEnd1 = minZ;
			diamEnd2 = maxZ;
		}

		// Initial sphere center and radius
		glm::vec3 center = (diamEnd1 + diamEnd2) * 0.5f;
		float radius = glm::length(diamEnd2 - center);

		// Step 3: Expand the sphere to include all vertices
		for (const auto& vertex : allVertices) {
			glm::vec3 vpos = { vertex.x,vertex.y,vertex.z };

			float distance = glm::length(vpos - center);
			if (distance > radius) {
				// Expand the sphere
				radius = (radius + distance) * 0.5f;
				center = center + (vpos - center) * (distance - radius) / distance;
			}
		}

		boundingSphere = glm::vec4(center, radius);
	//	return { center, radius };
	//}
}

GeometryAsset::GeometryAsset()
{

}

GeometryAsset::~GeometryAsset()
{

}

void GeometryAsset::DrawPreview()
{
	ImGui::Text("Triangles: %d Vertices: %d", (numIndices / 3), numVertices);

	ImGui::Checkbox("IsStatic", &isStaticPreview);

	const char* modes[] = { "UV","Normals","Vertex Color","Solid Color" };

	ImGui::Combo("Mode", &currentMode, modes, std::size(modes));
}

void GeometryAsset::RegeneratePreview()
{
	RecalculateBoundingSphere();

	//previewMesh
	{
		ermy::rendering::BufferDesc desc;
		desc.size = numIndices * sizeof(ermy::u16);
		desc.initialData = allIndices.data();
		desc.usage = BufferUsage::Index;
		previewMesh.indexBuffer = ermy::rendering::CreateDedicatedBuffer(desc);
	}

	{
		ermy::rendering::BufferDesc desc;
		desc.size = numVertices * sizeof(ermy::rendering::StaticVertexDedicated);
		desc.initialData = allVertices.data();
		desc.usage = BufferUsage::Vertex;
		previewMesh.vertexBuffer = ermy::rendering::CreateDedicatedBuffer(desc);
	}
	
	previewMesh.subMeshes = subMeshes;

	ermy::rendering::TextureDesc descStatic;
	descStatic.width = 128;
	descStatic.height = 128;
	descStatic.depth = 1;
	descStatic.isCubemap = false;
	descStatic.numLayers = 1;
	descStatic.numMips = 1;
	descStatic.isSparse = false;
	descStatic.texelFormat = ermy::rendering::Format::RGBA8_UNORM;

	descStatic.pixelsData = nullptr;
	descStatic.dataSize = 0;

	previewTextureStatic = ermy::rendering::CreateDedicatedTexture(descStatic);
	assetPreviewTexLive = ermy::rendering::GetTextureDescriptor(previewTextureLive);
	assetPreviewTexStatic = ermy::rendering::GetTextureDescriptor(previewTextureStatic);

	PreviewRenderer::Instance().EnqueueStaticPreviewGeneration(this);
}

void GeometryAsset::ResetView()
{
	Zoom = 1.0f;
	Yaw = Pitch = OldYaw = OldPitch = 0.0f;
	isPreviewDragging = false;
}

void GeometryAsset::MouseZoom(float value)
{
	Zoom /= value;
}
void GeometryAsset::MouseDown(float normalizedX, float normalizedY)
{
	if (!isPreviewDragging)
	{
		isPreviewDragging = true;
		OldYaw = Yaw;
		OldPitch = Pitch;
	}

}
void GeometryAsset::MouseUp()
{
	isPreviewDragging = false;
}

void GeometryAsset::MouseMove(float normalizedDeltaX, float normalizedDeltaY)
{
	if (isPreviewDragging)
	{
		Yaw = OldYaw - (normalizedDeltaX * 3.1415f);
		Pitch = OldPitch - (normalizedDeltaY * 3.1415f);
	}
}

void GeometryAsset::RenderStaticPreview(ermy::rendering::CommandList& cl)
{
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	cl.BeginRenderPass(staticRTT);
	GeometryRenderPreview::Instance().BindCheckerPSO(cl);
	cl.Draw(3);

	GeometryRenderPreview::Instance().BindPSO(cl,true);

	float r = boundingSphere.w;
	glm::vec3 C = glm::vec3(boundingSphere);

	float scale = 1.0f / r; // Scale to fit the bounding sphere

	//glm::translate(glm::mat4(1.0f), C) 
	glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.5f);

	//glm::mat4 viewMatrix = glm::rotate(glm::mat4(1.0f), Pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
	//	glm::rotate(glm::mat4(1.0f), Yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
	//	glm::translate(glm::mat4(1.0f), -cameraPos);

	glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), cameraPos);
	glm::mat4 projectionMatrix = glm::perspectiveLH_ZO(glm::radians(45.0f), 1.0f, 0.01f, 100.0f);

	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
	// * viewMatrix * modelMatrix

	const int UVMode = 0;
	cl.SetRootConstant(UVMode, ShaderStage::Fragment, 64);
	cl.DrawDedicatedMesh(previewMesh, glm::transpose(MVP));
	
	cl.EndRenderPass();

	auto staticRTTTex = PreviewRenderer::Instance().GetStaticTexture();
	cl.BlitTexture(staticRTTTex, previewTextureStatic);
	int a = 42;


}

void GeometryAsset::RenderPreview(ermy::rendering::CommandList& cl)
{
	GeometryRenderPreview::Instance().BindCheckerPSO(cl);
	cl.Draw(3);

	GeometryRenderPreview::Instance().BindPSO(cl, false);

	float r = boundingSphere.w;
	glm::vec3 C = glm::vec3(boundingSphere);

	float scale = 1.0f / r; // Scale to fit the bounding sphere

	//glm::translate(glm::mat4(1.0f), C) 
	glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), Pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), Yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);

	//glm::mat4 viewMatrix = glm::rotate(glm::mat4(1.0f), Pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
	//	glm::rotate(glm::mat4(1.0f), Yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
	//	glm::translate(glm::mat4(1.0f), -cameraPos);

	glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), cameraPos);
	glm::mat4 projectionMatrix = glm::perspectiveLH_ZO(glm::radians(45.0f / Zoom), 1.0f, 0.01f, 100.0f);

	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
	// * viewMatrix * modelMatrix
	cl.SetRootConstant(currentMode, ShaderStage::Fragment,64);
	cl.DrawDedicatedMesh(previewMesh, glm::transpose(MVP));
}
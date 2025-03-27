#include <assets/geometry_asset.h>
#include <ermy_log.h>
#include <preview_renderer.h>
#include <editor_shader_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ermy_application.h>

//Assimp::Importer* gAssetImporter = nullptr;
using namespace ermy;
using namespace ermy::rendering;

class GeometryRenderPreview
{
	GeometryRenderPreview();
	~GeometryRenderPreview();

	rendering::PSOID fullscreenEmpty;
	rendering::PSOID renderMesh;
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
		cl.SetPSO(renderMesh);
	}
};

GeometryRenderPreview::GeometryRenderPreview()
{
	auto const& renderCfg = ermy::Application::GetApplication()->staticConfig.render;

	auto RTT = PreviewRenderer::Instance().GetRTT();
	auto staticRTT = PreviewRenderer::Instance().GetStaticRTT();

	//live preview PSO
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::fullscreenVS());
		desc.SetShaderStage(ermy::shader_editor::fullscreenFSEmpty());
		desc.specificRenderPass = RTT;
		desc.debugName = "FullscreenEmpty";
		fullscreenEmpty = rendering::CreatePSO(desc);
	}	

	//render mesh PSO
	{
		rendering::PSODesc desc;
		desc.SetShaderStage(ermy::shader_editor::dedicatedStaticMeshVS());

		//if support barycentric
		if (renderCfg.enableBarycentricFS)
		{
			desc.SetShaderStage(ermy::shader_editor::dedicatedStaticMeshBaryFS());
		}
		else
		{
			if (renderCfg.enableGeometryShader) //support geometry shader
			{
				desc.SetShaderStage(ermy::shader_editor::dedicatedStaticMeshGeomBaryFS());
				desc.SetShaderStage(ermy::shader_editor::generateBarycentricGS());
			}
			else
			{
				desc.SetShaderStage(ermy::shader_editor::dedicatedStaticMeshFS());
			}
		}
		


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
		desc.debugName = "RenderMesh";
		renderMesh = rendering::CreatePSO(desc);
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

void GeometryAsset::RegenerateLivePreview()
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
	descStatic.texelSourceFormat = ermy::rendering::Format::RGBA8_UNORM;

	descStatic.pixelsData = nullptr;
	descStatic.dataSize = 0;

	//previewTextureStatic = ermy::rendering::CreateDedicatedTexture(descStatic);
	//assetPreviewTexStatic = ermy::rendering::GetTextureDescriptor(previewTextureStatic);

	PreviewRenderer::Instance().EnqueueStaticPreviewGeneration(this);
}

void GeometryAsset::ResetView()
{
	Zoom = 1.0f;
	rotationQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
	lastMousePos = glm::vec2(0.0f);
}

void GeometryAsset::MouseZoom(float value)
{
	Zoom *= value;
}
void GeometryAsset::MouseDown(float normalizedX, float normalizedY, int button)
{
	if (button == 0)
	{
		lastMousePos = glm::vec2(normalizedX, normalizedY);
		lastRotationQuat = rotationQuat;
	}

	if (button == 1)
	{
		oldCamPosition = cameraPos;
	}
}
void GeometryAsset::MouseUp(int button)
{

}

// Helper: Project 2D normalized coordinates onto a unit sphere
glm::vec3 projectToSphere(glm::vec2 screenPos) {
	// Convert normalized screen coordinates (-1 to 1 range expected)
	float x = screenPos.x * 2.0f - 1.0f; // Map [0,1] to [-1,1]
	float y = -(screenPos.y * 2.0f - 1.0f); // Flip Y for typical screen coords (top-left origin)

	float radius = 1.0f; // Virtual sphere radius
	float d = x * x + y * y;
	float t = radius * radius;

	// If point is outside the sphere, project onto the sphere surface
	if (d > t) {
		float s = radius / sqrt(d);
		return glm::vec3(x * s, y * s, 0.0f);
	}
	// If inside, compute Z to lie on the sphere
	else {
		return glm::vec3(x, y, sqrt(t - d));
	}
}

// Helper: Compute quaternion from arc between two points on a sphere
glm::quat quatFromArc(glm::vec3 from, glm::vec3 to) {
	from = glm::normalize(from);
	to = glm::normalize(to);

	float dot = glm::dot(from, to);
	if (dot > 0.99999f) { // Nearly parallel, no rotation
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}
	if (dot < -0.99999f) { // 180-degree rotation, pick an arbitrary axis
		glm::vec3 axis = glm::cross(from, glm::vec3(0.0f, 1.0f, 0.0f));
		if (glm::length(axis) < 0.0001f) axis = glm::vec3(1.0f, 0.0f, 0.0f);
		return glm::angleAxis(glm::pi<float>(), glm::normalize(axis));
	}

	glm::vec3 axis = glm::cross(from, to);
	float angle = acos(dot);
	return glm::angleAxis(angle, glm::normalize(axis));
}

void GeometryAsset::MouseMove(float normalizedX, float normalizedY, int button)
{
	if (button == 0)
	{
		glm::vec2 delta(-normalizedX, normalizedY);
		glm::vec2 currentMousePos = lastMousePos + delta;

		if (glm::length(delta) > 0.001f) { // Avoid tiny movements
			// Project 2D mouse positions onto a virtual sphere
			glm::vec3 from = projectToSphere(lastMousePos);
			glm::vec3 to = projectToSphere(currentMousePos);

			// Compute the rotation quaternion
			glm::quat deltaRot = quatFromArc(from, to);
			rotationQuat = glm::normalize(deltaRot * lastRotationQuat); // Apply new rotation to current orientation
			//rotationQuat = rotationQuat); // Ensure quaternion remains unit length

			//lastMousePos = currentMousePos;
		}
	}

	if (button == 1)
	{
		cameraPos = oldCamPosition + glm::vec3(normalizedX * 1.0f / Zoom, normalizedY * 1.0f / Zoom, 0);
	}
	//if (isPreviewDragging) {
	//	// Scale deltas for sensitivity (adjust as needed)
	//	float sensitivity = 2.0f; // Controls rotation speed
	//	float dx = normalizedDeltaX * sensitivity;
	//	float dy = normalizedDeltaY * sensitivity;

	//	// Avoid tiny movements
	//	if (fabs(dx) < 0.001f && fabs(dy) < 0.001f) {
	//		return;
	//	}

	//	// Compute rotation axis and angle from deltas
	//	// Horizontal delta (dx) rotates around Y-axis, vertical delta (dy) rotates around X-axis
	//	glm::vec3 axis = glm::normalize(glm::vec3(-dy, dx, 0.0f)); // X from dy, Y from dx, Z stays 0
	//	float angle = sqrt(dx * dx + dy * dy); // Magnitude of rotation

	//	// Create quaternion for this frame's rotation
	//	glm::quat deltaRot = glm::angleAxis(angle, axis);

	//	// Apply to the current rotation
	//	rotationQuat = deltaRot * rotationQuat;
	//	rotationQuat = glm::normalize(rotationQuat); // Ensure unit length
	//}
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
	//cl.BlitTexture(staticRTTTex, previewTextureStatic);
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
	glm::mat4 modelMatrix = glm::toMat4 (rotationQuat) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

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

void GeometryAsset::Save(pugi::xml_node& node)
{
	AssetData::Save(node);
}

void GeometryAsset::Load(pugi::xml_node& node)
{
	AssetData::Load(node);
}

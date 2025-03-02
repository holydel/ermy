#pragma once

#include <ermy_rendering.h>
#include <ermy_geometry.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
namespace ermy
{
	namespace scene
	{
		struct Transform
		{
			glm::vec4 position_uniform_scale = glm::vec4(0,0,0,1);
			glm::quat orientation = glm::identity<glm::quat>();
		};

		typedef Handle16 SceneID;
		typedef Handle32 GeometryID;
		typedef Handle32 EntityID;

		struct SceneDesc
		{
			u32 reserveTransformsCount = 65536;

			u32 totalStaticVertices = 8_MB;
			u32 totalSkinnedVertices = 1_MB;
			u32 totalStaticIndices = 4_MB;
			u32 totalSkinnedIndices = 2_MB;
		};

		void LoadFromPak(SceneID sceneId);
		void Unload(SceneID sceneId);

		SceneID Create(const SceneDesc& desc = SceneDesc());

		void SetActive(SceneID sceneId, bool status);
		void SetCurrent(SceneID sceneId);

		GeometryID LoadSubmesh(int numVertices, int numIndices, rendering::StaticVertexDedicated* vertices, u16* indices);

		inline GeometryID LoadSubmesh(const geometry::GeometryData& geoData)
		{
			return LoadSubmesh(geoData.numVertices, geoData.numIndices, geoData.verticecs, geoData.indices);
		}

		void SetCameraPosition(const glm::vec3& position);
		void SetCameraOrientation(const glm::quat& orientation);
		void SetCameraVerticalFOV(float fovRadians);

		void SetSkyBoxTexture(rendering::TextureID texture);

		EntityID AddEntity(GeometryID geom, const Transform& initialTransform = {});
		Transform& GetTransform(EntityID ent);
	}
}
#pragma once
#include "ermy_rendering.h"

namespace ermy
{
	namespace geometry
	{
		struct GeometryData
		{
			int numVertices;
			int numIndices;
			rendering::StaticVertexDedicated* verticecs;
			u16* indices;
		};

		GeometryData CreateCubeGeometry(float size);
		GeometryData CreateGeoSphereGeometry(int numSubdiv, float radius);
	}
}
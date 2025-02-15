#pragma once
#include "ermy_rendering.h"
#include <glm/glm.hpp>

namespace ermy
{
	namespace rendering
	{
		struct CommandList
		{
		private:
			void* nativeHandle = nullptr;

		public:
			CommandList(void* native)
				:nativeHandle(native)
			{

			}

			//GAPI_rendering - implementation
			void SetPSO(PSOID pso);
			void Draw(int numVertices, int numInstances = 1);
			void DrawIndexed(int numIndices, int numInstances = 1);

			void SetViewport(int x, int y, int width, int height);
			void SetScissor(int x, int y, int width, int height);

			void SetRootConstants(void* data, int size);

			void InsertDebugMark(const char* u8mark);
			void BeginDebugScope(const char* u8mark);
			void EndDebugScope();

			void BeginRenderPass(RenderPassID rtt, glm::vec4 clearColor = glm::vec4(0,0,0,0));
			void EndRenderPass();
		};
	}
}
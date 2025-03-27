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
			void* GetNativeHandle()
			{
				return nativeHandle;
			}

			//GAPI_rendering - implementation
			void SetPSO(PSOID pso);
			void Draw(int numVertices, int numInstances = 1);
			void DrawIndexed(int numIndices, int numInstances = 1);
			void SetDescriptorSet(int set, u64 handle);
			void SetViewport(int x, int y, int width, int height);
			void SetScissor(int x, int y, int width, int height);

			void SetRootConstants(const void* data, int size, ShaderStage stage = ShaderStage::Vertex, int offset = 0);

			template <typename T>
			void SetRootConstant(const T& data, ShaderStage stage = ShaderStage::Vertex, int offset = 0)
			{
				SetRootConstants(&data, sizeof(T), stage, offset);
			}

			void InsertDebugMark(const char* u8mark);
			void BeginDebugScope(const char* u8mark);
			void EndDebugScope();

			void BeginRenderPass(RenderPassID rtt, glm::vec4 clearColor = glm::vec4(0,0,0,0));
			void EndRenderPass();

			void BlitTexture(TextureID src, TextureID dest);

			void DrawDedicatedMesh(const DedicatedMesh& mesh, const glm::mat4& MVP);
			void SetVertexStream(BufferID buf);
			void SetIndexStream(BufferID buf);


			void UpdateBuffer(BufferID buf, const void* data, u16 size, u32 offset = 0); //max 65536 bytes
			void UpdateTexture(TextureID tex, const void* data);
			void ReadbackTexture(TextureID tex, void* data);
		};

		class OneTimeSubmitCommandList
		{
			void* pGAPIImpl = nullptr;

			OneTimeSubmitCommandList() = default;
			public:
				
				~OneTimeSubmitCommandList() = default;
			static OneTimeSubmitCommandList Allocate();
			void Submit();
			void WaitForCompletion();

			CommandList GetCL();
		};
	}
}
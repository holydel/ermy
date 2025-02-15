#pragma once
#include <ermy_api.h>
#include <vector>
#include <ermy_shader.h>

namespace ermy
{
	namespace rendering
	{
		enum class ShaderUniformType
		{
			Texture2D,
			Texture2DArray,
			TextureCube,
			TextureVolume,
		};

		enum class PrimitiveTopology
		{
			TriangleList,
			TriangleStrip,
			Points,
			LineList,
			LineStrip
		};

		typedef Handle32 PSOID;
		typedef Handle32 TextureID;
		typedef Handle32 BufferID;
		typedef Handle16 RenderPassID;

		struct PSODesc
		{
			int numRootConstants = 0;
			std::vector<ShaderBytecode> shaders;
			PrimitiveTopology topology = PrimitiveTopology::TriangleList;

			std::vector<ShaderUniformType> uniforms;
			const char* debugName = nullptr;

			RenderPassID specificRenderPass;
		};

		struct TextureDesc
		{
			u16 width = 1;
			u16 height = 1;
			u16 depth = 1;
			u16 numLayers =1;
			u8 numMips = 1;
			bool isCubemap : 1 = false;
			bool isSparse : 1 = false;

			void* pixelsData = nullptr;
			const char* debugName = nullptr;
		};

		struct BufferDesc
		{
			u64 size;
			const char* debugName = nullptr;
		};

		struct RenderPassDesc
		{
			TextureID colorAttachment;
			TextureID depthStencilAttachment;
		};

		PSOID CreatePSO(const PSODesc &desc);

		TextureID CreateDedicatedTexture(const TextureDesc &desc);
		BufferID CreateDedicatedBuffer(const BufferDesc &desc);

		ermy::u64 GetTextureDescriptor(TextureID tid); //ImTexture

		RenderPassID CreateRenderPass(const RenderPassDesc &desc);
		
	}
}
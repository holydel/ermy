#pragma once
#include <ermy_api.h>
#include <vector>
#include <ermy_shader.h>

namespace ermy
{
	namespace rendering
	{
		struct StaticVertexDedicated
		{
			float x, y, z;
			float nx, ny, nz;
			float tx, ty, tz;
			float bx, by, bz;
			float u0, v0;
			float u1, v1;
			float r, g, b, a;
		};

		struct SkinnedVertexDedicated
		{
			float x, y, z;
			float nx, ny, nz;
			float tx, ty, tz;
			float bx, by, bz;
			float u0, v0;
			float u1, v1;
			float r, g, b, a;
			float w0, w1, w2, w3;
			u32 boneIndices;
		};

		enum class Format
		{
			RGBA8_UNORM
			,RGBA16_UNORM
			,RGBA16_NORM
			,RGBA16F
			, RGBA16_UINT
			, RG16_UINT
			, R16_UINT
			, RG16_UNORM
			, R16_UNORM
			, RGBA8_UINT
			, R32F
			, RG32F
			, RGB32F
			, RGBA32F
		};

		struct FormatInfo
		{
			int size;
		};

		enum class TextureType
		{
			Tex2D
			,Tex3D
			,TexCube
			,TexArray2D
			,TexArrayCube
		};

		FormatInfo GetFormatInfo(Format format);

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

		struct VertexAttribute
		{
			Format format;
		};

		enum class BufferUsage
		{
			Vertex,
			Index,
			Uniform,
			Storage
		};

		typedef Handle32 PSOID;
		typedef Handle32 TextureID;
		typedef Handle32 BufferID;
		typedef Handle16 RenderPassID;

		struct RootConstantRange
		{
			u8 offset = 0;
			u8 size = 0;
		};

		struct PSODesc
		{
			//int numRootConstants = 0;			
			RootConstantRange rootConstantRanges[(int)ShaderStage::MAX] = {};
			std::vector<ShaderBytecode> shaders;
			PrimitiveTopology topology = PrimitiveTopology::TriangleList;

			std::vector<ShaderUniformType> uniforms;
			std::vector<VertexAttribute> vertexAttributes;
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
			Format texelFormat = Format::RGBA8_UNORM;
			u32 dataSize = 0;

			void* pixelsData = nullptr;
			const char* debugName = nullptr;
		};

		struct BufferDesc
		{
			u64 size = 0;
			BufferUsage usage = BufferUsage::Uniform;
			void* initialData = nullptr;
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
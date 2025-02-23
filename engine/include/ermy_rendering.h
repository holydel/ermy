#pragma once
#include <ermy_api.h>
#include <vector>
#include <ermy_shader.h>
#include <array>
#include <ermy_utils.h>

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
			, BC1
			,BC2
			,BC3
			,BC4
			,BC5
			,BC6
			,BC7
			,R8_UNORM
			,BGRA8_UNORM
			, D32F
			, D16_UNORM			
			, D24_UNORM_S8_UINT
		};

		struct FormatInfo
		{
			u8 blockSize;
			u8 channelsCount;
			u8 blockWidth;
			u8 blockHeight;
			u8 blockDepth;
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
		bool IsDepthFormat(Format format);

		enum class ShaderUniformType
		{
			Texture2D,
			Texture2DArray,
			TextureCube,
			TextureVolume,
			TextureCubeArray,
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
			std::array<ShaderInfo, (int)ShaderStage::MAX> allShaderStages;

			PrimitiveTopology topology = PrimitiveTopology::TriangleList;

			std::vector<ShaderUniformType> uniforms;
			std::vector<VertexAttribute> vertexAttributes;
			const char* debugName = nullptr;

			RenderPassID specificRenderPass;

			bool writeDepth : 1 = false;
			bool testDepth : 1 = false;

			void SetShaderStageInternal(const ShaderInfo& info)
			{
				allShaderStages[(int)info.byteCode.stage] = info;
			}

			void SetShaderStage(const ShaderInfo& info)
			{
				SetShaderStageInternal(info);
			}

			void SetShaderStage(const ShaderBytecode& bytecode, const std::string& shaderName = "")
			{
				int stageIndex = (int)bytecode.stage;

				allShaderStages[stageIndex].byteCode = bytecode;
				allShaderStages[stageIndex].shaderName = shaderName;
				allShaderStages[stageIndex].tag = ShaderDomainTag::Runtime;
				allShaderStages[stageIndex].bytecodeCRC64 = ermy_utils::hash::CalculateCRC64(bytecode.data, bytecode.size);
			}

			void AddRootConstantRange(ShaderStage stage, int size)
			{
				int maxEndOffset = 0;

				for (int i = 0; i < std::size(rootConstantRanges); ++i)
				{
					maxEndOffset = std::max(maxEndOffset,rootConstantRanges[i].offset + rootConstantRanges[i].size);
				}

				rootConstantRanges[(int)stage].offset = maxEndOffset;
				rootConstantRanges[(int)stage].size = size;
			}
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

		void UpdateShaderBytecode(ShaderDomainTag tag, const std::string& name, u64 bytecodeCRC, const u8* bytecode, u32 bytecodeSize);
	}
}
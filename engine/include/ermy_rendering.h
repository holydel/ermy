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
		typedef Handle32 PSOID;
		typedef Handle32 TextureID;
		typedef Handle32 BufferID;
		typedef Handle16 RenderPassID;

		struct SubMesh
		{
			u32 indexOffset;
			u32 vertexOffset;
			u32 indexCount;
			u32 vertexCount;
		};

		struct DedicatedMesh
		{
			BufferID indexBuffer;
			BufferID vertexBuffer;

			std::vector<SubMesh> subMeshes;
		};

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

		enum class Format : u8
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

		enum class ShaderUniformType : u8
		{
			Texture2D,
			Texture2DArray,
			TextureCube,
			TextureVolume,
			TextureCubeArray,
			UniformBuffer,
			StorageBuffer,
			StorageImage
		};

		enum class PrimitiveTopology : u8
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

		enum class BufferUsage : u8
		{
			Vertex,
			Index,
			Uniform,
			Storage
		};

		struct RootConstantRange
		{
			u8 offset = 0;
			u8 size = 0;
		};

		enum class PSODomain : u8
		{
			None,
			Canvas,
			Scene
		};

		struct PSODesc
		{
			//int numRootConstants = 0;			
			RootConstantRange rootConstantRanges[(int)ShaderStage::MAX] = {};
			std::array<ShaderInfo, (int)ShaderStage::MAX> allShaderStages;

			PrimitiveTopology topology = PrimitiveTopology::TriangleList;

			PSODomain domain = PSODomain::None;

			std::vector<ShaderUniformType> uniforms;
			std::vector<VertexAttribute> vertexAttributes;
			const char* debugName = nullptr;

			RenderPassID specificRenderPass;

			bool writeDepth : 1 = false;
			bool testDepth : 1 = false;
			bool isStereo : 1 = false;

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
			void* pixelsData = nullptr;
			const char* debugName = nullptr;
			u32 dataSize = 0;
			u16 width = 1;
			u16 height = 1;
			u16 depth = 1;
			u16 numLayers =1;
			u8 numMips = 1;
			Format texelFormat = Format::RGBA8_UNORM;
			bool isCubemap : 1 = false;
			bool isSparse : 1 = false;
		};

		struct BufferDesc
		{
			u64 size = 0;
			BufferUsage usage = BufferUsage::Uniform;
			void* initialData = nullptr;
			const char* debugName = nullptr;

			bool persistentMapped = false;
		};

		struct RenderPassDesc
		{
			TextureID colorAttachment;
			TextureID depthStencilAttachment;
		};

		struct DescriptorSetDesc
		{
			struct Binding
			{
				u8 bindingSlot;
				ShaderUniformType uniformType;

				union
				{
					TextureID texture;
					BufferID uniformBuffer;
				};
			};

			std::vector<Binding> allBindings;

			void AddBindingImpl(ShaderUniformType type, auto&& setter)
			{
				Binding binding{};
				binding.bindingSlot = static_cast<u8>(allBindings.size());
				binding.uniformType = type;
				setter(binding);
				allBindings.push_back(binding);
			}

			void AddBindingTexture2D(TextureID tex)
			{
				AddBindingImpl(ShaderUniformType::Texture2D, [tex](Binding& b) {
					b.texture = tex;
				});
			}

			void AddBindingTextureCube(TextureID tex)
			{
				AddBindingImpl(ShaderUniformType::TextureCube, [tex](Binding& b) {
					b.texture = tex;
				});
			}

			void AddBindingUniformBuffer(BufferID buffer)
			{
				AddBindingImpl(ShaderUniformType::UniformBuffer, [buffer](Binding& b) {
					b.uniformBuffer = buffer;
				});
			}
		};

		PSOID CreatePSO(const PSODesc &desc);

		TextureID CreateDedicatedTexture(const TextureDesc &desc);
		BufferID CreateDedicatedBuffer(const BufferDesc &desc);
		void UpdateBufferData(BufferID buffer, const void* data);
		ermy::u64 GetTextureDescriptor(TextureID tid); //ImTexture
		ermy::u64 GetBufferDescriptor(BufferID tid); //ImTexture

		RenderPassID CreateRenderPass(const RenderPassDesc &desc);		

		void UpdateShaderBytecode(ShaderDomainTag tag, const std::string& name, u64 bytecodeCRC, const u8* bytecode, u32 bytecodeSize);

		void UpdateBufferData(BufferID buffer, u32 offset, void* data, u16 dataSize);

		ermy::u64 CreateDescriptorSet(PSODomain domain, const DescriptorSetDesc& desc);
		void UpdateDescriptorSet(ermy::u64 ds, const DescriptorSetDesc::Binding& binding);

		void WaitDeviceIdle();
	}
}
#pragma once
#include <ermy_api.h>
#include <vector>
#include <ermy_shader.h>

namespace ermy
{
	namespace rendering
	{
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

		struct PSODesc
		{
			int numRootConstants = 0;
			std::vector<ShaderBytecode> shaders;
			PrimitiveTopology topology = PrimitiveTopology::TriangleList;
		};

		struct TextureDesc
		{
			u16 width;
			u16 height;
			u16 depth;
			u16 numLayers;
			u8 numMips;
			bool isCubemap : 1;
			bool isSparse : 1;

			void* pixelsData;
		};

		struct BufferDesc
		{
			u64 size;
		};
		PSOID CreatePSO(const PSODesc &desc);

		TextureID CreateDedicatedTexture(const TextureDesc &desc);
		BufferID CreateDedicatedBuffer(const BufferDesc &desc);
	}
}
#include "compressonator_lib.h"

using namespace ermy;
using namespace rendering;

CMP_ChannelFormat CMP_GetChannelFormat(CMP_FORMAT format) {
	switch (format) {
		// 8-bit integer formats (CF_8bit)
	case CMP_FORMAT_RGBA_8888_S:
	case CMP_FORMAT_ARGB_8888_S:
	case CMP_FORMAT_ARGB_8888:
	case CMP_FORMAT_ABGR_8888:
	case CMP_FORMAT_RGBA_8888:
	case CMP_FORMAT_BGRA_8888:
	case CMP_FORMAT_RGB_888:
	case CMP_FORMAT_RGB_888_S:
	case CMP_FORMAT_BGR_888:
	case CMP_FORMAT_RG_8_S:
	case CMP_FORMAT_RG_8:
	case CMP_FORMAT_R_8_S:
	case CMP_FORMAT_R_8:
		return CF_8bit;

		// 16-bit integer formats (CF_16bit)
	case CMP_FORMAT_ARGB_16:
	case CMP_FORMAT_ABGR_16:
	case CMP_FORMAT_RGBA_16:
	case CMP_FORMAT_BGRA_16:
	case CMP_FORMAT_RG_16:
	case CMP_FORMAT_R_16:
		return CF_16bit;

		// 10-10-10-2 formats (CF_2101010 or CF_1010102)
	case CMP_FORMAT_ARGB_2101010:
		return CF_2101010;
	case CMP_FORMAT_RGBA_1010102:
		return CF_1010102;

		// 16-bit float formats (CF_Float16)
	case CMP_FORMAT_ARGB_16F:
	case CMP_FORMAT_ABGR_16F:
	case CMP_FORMAT_RGBA_16F:
	case CMP_FORMAT_BGRA_16F:
	case CMP_FORMAT_RG_16F:
	case CMP_FORMAT_R_16F:
		return CF_Float16;

		// 32-bit float formats (CF_Float32)
	case CMP_FORMAT_ARGB_32F:
	case CMP_FORMAT_ABGR_32F:
	case CMP_FORMAT_RGBA_32F:
	case CMP_FORMAT_BGRA_32F:
	case CMP_FORMAT_RGB_32F:
	case CMP_FORMAT_BGR_32F:
	case CMP_FORMAT_RG_32F:
	case CMP_FORMAT_R_32F:
		return CF_Float32;

		// RGBE format (CF_Float9995E)
	case CMP_FORMAT_RGBE_32F:
		return CF_Float9995E;

		// Compressed formats (CF_Compressed)
	case CMP_FORMAT_BC1:
	case CMP_FORMAT_BC2:
	case CMP_FORMAT_BC3:
	case CMP_FORMAT_BC4:
	case CMP_FORMAT_BC4_S:
	case CMP_FORMAT_BC5:
	case CMP_FORMAT_BC5_S:
	case CMP_FORMAT_BC6H:
	case CMP_FORMAT_BC6H_SF:
	case CMP_FORMAT_BC7:
	case CMP_FORMAT_ATI1N:
	case CMP_FORMAT_ATI2N:
	case CMP_FORMAT_ATI2N_XY:
	case CMP_FORMAT_ATI2N_DXT5:
	case CMP_FORMAT_DXT1:
	case CMP_FORMAT_DXT3:
	case CMP_FORMAT_DXT5:
	case CMP_FORMAT_DXT5_xGBR:
	case CMP_FORMAT_DXT5_RxBG:
	case CMP_FORMAT_DXT5_RBxG:
	case CMP_FORMAT_DXT5_xRBG:
	case CMP_FORMAT_DXT5_RGxB:
	case CMP_FORMAT_DXT5_xGxR:
	case CMP_FORMAT_ATC_RGB:
	case CMP_FORMAT_ATC_RGBA_Explicit:
	case CMP_FORMAT_ATC_RGBA_Interpolated:
	case CMP_FORMAT_ASTC:
	case CMP_FORMAT_APC:
	case CMP_FORMAT_PVRTC:
	case CMP_FORMAT_ETC_RGB:
	case CMP_FORMAT_ETC2_RGB:
	case CMP_FORMAT_ETC2_SRGB:
	case CMP_FORMAT_ETC2_RGBA:
	case CMP_FORMAT_ETC2_RGBA1:
	case CMP_FORMAT_ETC2_SRGBA:
	case CMP_FORMAT_ETC2_SRGBA1:
	case CMP_FORMAT_GTC:
	case CMP_FORMAT_BASIS:
		return CF_Compressed;

		// Lossless compression format
	case CMP_FORMAT_BROTLIG:
		return CF_Compressed;

		// Binary format (treated as 8-bit untyped data)
	case CMP_FORMAT_BINARY:
		return CF_8bit;

		// Unknown format
	case CMP_FORMAT_Unknown:
	case CMP_FORMAT_MAX:
	default:
		return CF_8bit;  // Default to 8-bit as a fallback
	}
}

CompressonatorLib::CompressonatorLib()
{
	CMP_InitFramework();
	CMP_InitializeBCLibrary();
}

CompressonatorLib::~CompressonatorLib()
{
	CMP_DestroyComputeLibrary(true);
}


CompressonatorLib& CompressonatorLib::Instance()
{
	static CompressonatorLib instance;
	return instance;
}

CMP_MipSet CompressonatorLib::Load(const char* utf8_file)
{
	CMP_MipSet result = {};
	auto errCode = CMP_LoadTexture(utf8_file, &result);
	return result;
}

CMP_ERROR CompressonatorLib::Save(const char* utf8_file, CMP_MipSet& mipSet)
{
	return CMP_SaveTexture(utf8_file, &mipSet);
}

void CompressonatorLib::RegenerateMips(CMP_MipSet& mipSet, int minSize)
{
	if (mipSet.m_nMipLevels > 1 || mipSet.m_ChannelFormat == CF_Compressed)
		return;

	// Calculate max possible mip levels
	int maxLevels = CMP_CalcMaxMipLevel(mipSet.m_nHeight, mipSet.m_nWidth, true);
		
	// Calculate minimum size for the requested mip levels
	int adjustedMinSize = CMP_CalcMinMipSize(mipSet.m_nHeight, mipSet.m_nWidth, maxLevels);
		
	// Use the larger of the requested and adjusted minimum sizes
	minSize = std::max(minSize, adjustedMinSize);
		
	// Generate mip levels
	CMP_GenerateMIPLevels(&mipSet, minSize);
	

	// Recreate mipset data for all mip levels
	CMP_DWORD totalSize = 0;
	for (CMP_INT mipLevel = 0; mipLevel < mipSet.m_nMipLevels; mipLevel++) {
		CMP_MipLevel* level = nullptr;
		CMP_GetMipLevel(&level, &mipSet, mipLevel, 0);
		if (level) {
			totalSize += level->m_dwLinearSize;
		}
	}

	// Allocate new buffer for all mip data
	CMP_BYTE* newData = (CMP_BYTE*)malloc(totalSize);
	CMP_DWORD offset = 0;

	// Copy data from each mip level
	for (CMP_INT mipLevel = 0; mipLevel < mipSet.m_nMipLevels; mipLevel++) {
		CMP_MipLevel* level = nullptr;
		CMP_GetMipLevel(&level, &mipSet, mipLevel, 0);
		if (level && level->m_pbData) {
			memcpy(newData + offset, level->m_pbData, level->m_dwLinearSize);
			offset += level->m_dwLinearSize;
		}
	}

	// Free old data and update mipset
	if (mipSet.pData) {
		free(mipSet.pData);
	}
	mipSet.pData = newData;
	mipSet.dwDataSize = totalSize;
}

extern float gCurrentTextureProgress;

CMP_ERROR CompressonatorLib::SetupCompressionOptions(KernelOptions& options, 
												   const CompressionSettings& settings,
												   ermy::rendering::Format targetFormat) {
	memset(&options, 0, sizeof(KernelOptions));
	
	options.format = ConvertFormatToCMPFormat(targetFormat);
	options.fquality = settings.quality;
	options.threads = settings.numThreads;
	options.encodeWith = settings.computeType;
	options.useSRGBFrames = settings.useSRGB;
	
	return CMP_OK;
}

GeneratedMipData CompressonatorLib::GenerateMipTexels2D(const u8* srcTexelData, u32 width, u32 height, ermy::rendering::Format format, u32 numLayers)
{
	GeneratedMipData result = {};

	auto finfo = rendering::GetFormatInfo(format);
	int layerSize = finfo.GetDataSize2D(width, height);

	int numMips = CMP_CalcMaxMipLevel(height, width, false);
	result.mipLevels = numMips;
	int fullLayerSize = layerSize;

	for (int i = 0; i < numMips; ++i)
	{
		u32 mipWidth = std::max(1u, width >> i);
		u32 mipHeight = std::max(1u, height >> i);
		fullLayerSize += finfo.GetDataSize2D(mipWidth, mipHeight);
	}

	result.size = fullLayerSize * numLayers;
	result.data = (u8*)malloc(result.size);


	CMP_FORMAT cmpFormat = ConvertFormatToCMPFormat(format);
	CMP_ChannelFormat channelFormat = CMP_GetChannelFormat(cmpFormat);
	CMP_CFilterParams filterParams = {};
	
	for (int i = 0; i < numLayers; ++i)
	{		
		const u8* srcData = srcTexelData + i * layerSize;
		CMP_MipSet  mipSet = {};
		
		CMP_CreateMipSet(& mipSet, width, height, 1, channelFormat, TT_2D);
		mipSet.m_format = cmpFormat;

		mipSet.m_pMipLevelTable[0]->m_pbData = (CMP_BYTE*)srcData;
		mipSet.m_pMipLevelTable[0]->m_dwLinearSize = layerSize;
		//for (int j = 0; j < numMips; ++j)
		//{
		//	mipSet.
		//}

		//int maxLevels = CMP_CalcMaxMipLevel(mipSet.m_nHeight, mipSet.m_nWidth, true);

		//// Calculate minimum size for the requested mip levels
		//int adjustedMinSize = CMP_CalcMinMipSize(mipSet.m_nHeight, mipSet.m_nWidth, maxLevels);

		//// Use the larger of the requested and adjusted minimum sizes
		//auto minSize = std::max(minSize, adjustedMinSize);


		CMP_GenerateMIPLevels(&mipSet, 1);

		// Copy all generated mip levels to result
		u8* destPtr = result.data + i * fullLayerSize;
		for (int j = 0; j < numMips; ++j)
		{
			CMP_MipLevel* mipLevel = nullptr;
			CMP_GetMipLevel(&mipLevel, &mipSet, j, 0);
			if (mipLevel)
			{
				u32 mipSize = finfo.GetDataSize2D(mipLevel->m_nWidth, mipLevel->m_nHeight);
				memcpy(destPtr, mipLevel->m_pbData, mipSize);
				destPtr += mipSize;
			}
		}
	}

	return result;
}

GeneratedMipData CompressonatorLib::GenerateMipTexels3D(const u8* srcTexelData, u32 width, u32 height, u32 depth, ermy::rendering::Format format)
{
	GeneratedMipData result = {};

	return result;
}

CMP_ERROR CompressonatorLib::CompressMips(CMP_MipSet& srcSet, 
										CMP_MipSet& dstSet,
										ermy::rendering::Format sourceFormat,
										ermy::rendering::Format targetFormat,
										const CompressionSettings& settings) {
	// Initialize destination mipset
	memset(&dstSet, 0, sizeof(CMP_MipSet));
	dstSet.m_nHeight = srcSet.m_nHeight;
	dstSet.m_nWidth = srcSet.m_nWidth;
	dstSet.m_nDepth = settings.isArrayTexture ? settings.arraySize : srcSet.m_nDepth;
	dstSet.m_TextureType = settings.isArrayTexture ? TT_VolumeTexture : srcSet.m_TextureType;
	dstSet.m_nMipLevels = srcSet.m_nMipLevels;
	dstSet.m_ChannelFormat = srcSet.m_ChannelFormat;
	dstSet.m_TextureDataType = srcSet.m_TextureDataType;
	dstSet.m_format = ConvertFormatToCMPFormat(targetFormat);

	// Create compressed mipset structure
	CMP_ERROR errCode = CMP_CreateCompressMipSet(&dstSet, &srcSet);
	if (errCode != CMP_OK) return errCode;

	// Setup compression options
	KernelOptions options;
	errCode = SetupCompressionOptions(options, settings, targetFormat);
	if (errCode != CMP_OK) return errCode;

	// Setup compute library if using GPU or HPC
	if (settings.computeType != CMP_CPU) {
		errCode = CMP_CreateComputeLibrary(&srcSet, &options, nullptr);
		if (errCode != CMP_OK) {
			// Fall back to CPU if compute library setup fails
			options.encodeWith = CMP_CPU;
		}
	}

	// Compress texture
	errCode = CMP_CompressTexture(&options, srcSet, dstSet, nullptr);
	
	return errCode;
}

CMP_ERROR CompressonatorLib::CreateBlockEncoder(void** encoder,
											  ermy::rendering::Format format,
											  const CompressionSettings& settings) {
	CMP_EncoderSetting encodeSettings = {};
	encodeSettings.format = ConvertFormatToCMPFormat(format);
	encodeSettings.quality = settings.quality;
	
	return CMP_CreateBlockEncoder(encoder, encodeSettings);
}

CMP_ERROR CompressonatorLib::CompressBlock(void** encoder,
										 void* srcBlock,
										 unsigned int srcStride,
										 void* dstBlock,
										 unsigned int dstStride) {
	return CMP_CompressBlock(encoder, srcBlock, srcStride, dstBlock, dstStride);
}

void CompressonatorLib::DestroyBlockEncoder(void** encoder) {
	CMP_DestroyBlockEncoder(encoder);
}

CMP_ERROR CompressonatorLib::AnalyzeTexture(CMP_MipSet& src1,
										  CMP_MipSet& src2,
										  CMP_AnalysisData& analysis) {
	analysis.analysisMode = CMP_ANALYSIS_MSEPSNR;
	analysis.channelBitMap = 0xF; // RGBA
	
	return CMP_MipSetAnlaysis(&src1, &src2, 0, 0, &analysis);
}

// Convert CMP_FORMAT to Format

Format CompressonatorLib::ConvertCMPFormatToFormat(CMP_FORMAT cmpFormat) {
	switch (cmpFormat) {
	case CMP_FORMAT_ARGB_8888:      return Format::ARGB8_UNORM;
	case CMP_FORMAT_RGBA_8888:      return Format::RGBA8_UNORM;
	case CMP_FORMAT_BGRA_8888:      return Format::BGRA8_UNORM;
	case CMP_FORMAT_RG_8:           return Format::RG8_UNORM;
	case CMP_FORMAT_R_8:            return Format::R8_UNORM;
	case CMP_FORMAT_RGBA_16:        return Format::RGBA16_UNORM;
	case CMP_FORMAT_RG_16:          return Format::RG16_UNORM;
	case CMP_FORMAT_R_16:           return Format::R16_UNORM;
	case CMP_FORMAT_RGBA_16F:       return Format::RGBA16F;
	case CMP_FORMAT_RG_16F:         return Format::RG16F;
	case CMP_FORMAT_R_16F:          return Format::R16F;
	case CMP_FORMAT_RGBA_32F:       return Format::RGBA32F;
	case CMP_FORMAT_RGB_32F:        return Format::RGB32F;
	case CMP_FORMAT_RG_32F:         return Format::RG32F;
	case CMP_FORMAT_R_32F:          return Format::R32F;
	case CMP_FORMAT_BC1:            return Format::BC1;
	case CMP_FORMAT_BC2:            return Format::BC2;
	case CMP_FORMAT_BC3:            return Format::BC3;
	case CMP_FORMAT_BC4:            return Format::BC4;
	case CMP_FORMAT_BC5:            return Format::BC5;
	case CMP_FORMAT_BC6H:           return Format::BC6;
	case CMP_FORMAT_BC6H_SF:        return Format::BC6_SF;
	case CMP_FORMAT_BC7:            return Format::BC7;
	case CMP_FORMAT_DXT1:           return Format::BC1;
	case CMP_FORMAT_DXT3:           return Format::BC2;
	case CMP_FORMAT_DXT5:           return Format::BC3;

	default:                        return Format::UNKNOWN;
	}
}


CMP_FORMAT CompressonatorLib::ConvertFormatToCMPFormat(Format format) {
	switch (format) {
	case Format::RGBA8_SRGB:
	case Format::RGBA8_UNORM:       return CMP_FORMAT_RGBA_8888;
	case Format::ARGB8_UNORM:       return CMP_FORMAT_ARGB_8888;
	case Format::BGRA8_UNORM:       return CMP_FORMAT_BGRA_8888;
	case Format::RG8_UNORM:         return CMP_FORMAT_RG_8;
	case Format::R8_UNORM:          return CMP_FORMAT_R_8;
	case Format::RGBA16_UNORM:      return CMP_FORMAT_RGBA_16;
	case Format::RG16_UNORM:        return CMP_FORMAT_RG_16;
	case Format::R16_UNORM:         return CMP_FORMAT_R_16;
	case Format::RGBA16F:           return CMP_FORMAT_RGBA_16F;
	case Format::RG16F:             return CMP_FORMAT_RG_16F;
	case Format::R16F:              return CMP_FORMAT_R_16F;
	case Format::RGBA32F:           return CMP_FORMAT_RGBA_32F;
	case Format::RGB32F:            return CMP_FORMAT_RGB_32F;
	case Format::RG32F:             return CMP_FORMAT_RG_32F;
	case Format::R32F:              return CMP_FORMAT_R_32F;
	case Format::BC1:               return CMP_FORMAT_BC1;
	case Format::BC2:               return CMP_FORMAT_BC2;
	case Format::BC3:               return CMP_FORMAT_BC3;
	case Format::BC4:               return CMP_FORMAT_BC4;
	case Format::BC5:               return CMP_FORMAT_BC5;
	case Format::BC6:               return CMP_FORMAT_BC6H;
	case Format::BC6_SF:            return CMP_FORMAT_BC6H_SF;
	case Format::BC7:               return CMP_FORMAT_BC7;
	case Format::D32F:              return CMP_FORMAT_R_32F;
	case Format::D16_UNORM:         return CMP_FORMAT_R_16;

	default:                        return CMP_FORMAT_Unknown;
	}
}

CMP_ERROR CompressonatorLib::InitializeArrayMipSet(CMP_MipSet& mipSet,
                                                  int width,
                                                  int height,
                                                  int arraySize,
                                                  ChannelFormat channelFormat,
                                                  CMP_FORMAT format) {
    memset(&mipSet, 0, sizeof(CMP_MipSet));
    
    mipSet.m_nWidth = width;
    mipSet.m_nHeight = height;
    mipSet.m_nDepth = arraySize;  // For array textures, depth represents array size
    mipSet.m_TextureType = TT_VolumeTexture;  // Use volume texture type for arrays
    mipSet.m_ChannelFormat = channelFormat;
    mipSet.m_format = format;
    
    return CMP_CreateMipSet(&mipSet, width, height, arraySize, channelFormat, TT_VolumeTexture);
}

CMP_ERROR CompressonatorLib::LoadArray(const char* utf8_file, CMP_MipSet& mipSet, int arraySize) {
    CMP_ERROR result = CMP_LoadTexture(utf8_file, &mipSet);
    if (result != CMP_OK) return result;
    
    // Validate and adjust for array texture
    if (mipSet.m_TextureType != TT_VolumeTexture) {
        mipSet.m_TextureType = TT_VolumeTexture;
        mipSet.m_nDepth = arraySize;
    }
    
    return CMP_OK;
}

CMP_ERROR CompressonatorLib::SaveArray(const char* utf8_file, const CMP_MipSet& mipSet) {
    // Ensure it's an array texture
    if (mipSet.m_TextureType != TT_VolumeTexture) {
        return CMP_ERR_INVALID_SOURCE_TEXTURE;
    }
    
    return CMP_SaveTexture(utf8_file, const_cast<CMP_MipSet*>(&mipSet));
}

void CompressonatorLib::RegenerateMipsForArray(CMP_MipSet& mipSet, int minSize) {
    if (mipSet.m_TextureType != TT_VolumeTexture) return;
    
    if (mipSet.m_nMipLevels <= 1) {
        // Calculate max possible mip levels (same for each array slice)
        int maxLevels = CMP_CalcMaxMipLevel(mipSet.m_nHeight, mipSet.m_nWidth, true);
        
        // Calculate minimum size for the requested mip levels
        int adjustedMinSize = CMP_CalcMinMipSize(mipSet.m_nHeight, mipSet.m_nWidth, maxLevels);
        
        // Use the larger of the requested and adjusted minimum sizes
        minSize = std::max(minSize, adjustedMinSize);
        
        // Generate mip levels for each array slice
        CMP_GenerateMIPLevels(&mipSet, minSize);
    }

    // Recreate mipset data for all mip levels and array slices
    CMP_DWORD totalSize = 0;
    for (CMP_INT arrayIndex = 0; arrayIndex < mipSet.m_nDepth; arrayIndex++) {
        for (CMP_INT mipLevel = 0; mipLevel < mipSet.m_nMipLevels; mipLevel++) {
            CMP_MipLevel* level = nullptr;
            CMP_GetMipLevel(&level, &mipSet, mipLevel, arrayIndex);
            if (level) {
                totalSize += level->m_dwLinearSize;
            }
        }
    }

    // Allocate new buffer for all mip data
    CMP_BYTE* newData = (CMP_BYTE*)malloc(totalSize);
    CMP_DWORD offset = 0;

    // Copy data from each mip level of each array slice
    for (CMP_INT arrayIndex = 0; arrayIndex < mipSet.m_nDepth; arrayIndex++) {
        for (CMP_INT mipLevel = 0; mipLevel < mipSet.m_nMipLevels; mipLevel++) {
            CMP_MipLevel* level = nullptr;
            CMP_GetMipLevel(&level, &mipSet, mipLevel, arrayIndex);
            if (level && level->m_pbData) {
                memcpy(newData + offset, level->m_pbData, level->m_dwLinearSize);
                offset += level->m_dwLinearSize;
            }
        }
    }

    // Free old data and update mipset
    if (mipSet.pData) {
        free(mipSet.pData);
    }
    mipSet.pData = newData;
    mipSet.dwDataSize = totalSize;
}
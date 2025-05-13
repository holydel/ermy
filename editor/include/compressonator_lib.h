#pragma once
#include <compressonator.h>
#include <ermy_rendering.h>

struct CompressionSettings {
	float quality = 1.0f;                    // Quality setting [0.0-1.0]
	CMP_Compute_type computeType = CMP_HPC;  // Default to HPC (vectorized CPU)
	int numThreads = 0;                      // 0 = Auto thread count
	bool generateMipMaps = true;             // Whether to generate mipmaps
	int minMipSize = 1;                      // Minimum size for generated mips
	bool useSRGB = false;                    // Use SRGB color space
	
	// BC7-specific settings
	bool bc7_restrictColour = false;         // Avoid using combined alpha modes
	bool bc7_restrictAlpha = false;          // Avoid issues with punch-through alpha
	float bc7_minThreshold = 5.0f;          // Min error threshold for BC7
	float bc7_maxThreshold = 80.0f;         // Max error threshold for BC7
	
	// Channel weights (for BC1-BC3)
	float weightR = 1.0f;
	float weightG = 1.0f;
	float weightB = 1.0f;
	
	// BC1-specific settings
	unsigned char alphaThreshold = 128;      // Alpha threshold for BC1

	// Array texture settings
	bool isArrayTexture = false;     // Whether this is a 2D array texture
	int arraySize = 1;              // Number of array layers (depth)
};

struct GeneratedMipData {
	ermy::u8* data = nullptr;
	size_t size = 0;
	int minWidth = 0;
	int minHeight = 0;
	int minDepth = 0;
	int mipLevels = 0;

	void Free()
	{
		if (data)
		{
			free(data);
			data = nullptr;
			size = 0;
		}
	}
};

class CompressonatorLib
{
	CompressonatorLib();
	~CompressonatorLib();
public:
	static CompressonatorLib& Instance();

	CMP_MipSet Load(const char* utf8_file);

	CMP_ERROR Save(const char* utf8_file, CMP_MipSet& mipSet);

	static CMP_FORMAT ConvertFormatToCMPFormat(ermy::rendering::Format format);
	static ermy::rendering::Format ConvertCMPFormatToFormat(CMP_FORMAT cmpFormat);

	GeneratedMipData GenerateMipTexels2D(const ermy::u8* srcTexelData, ermy::u32 width, ermy::u32 height, ermy::rendering::Format format, ermy::u32 numLayers = 1);
	GeneratedMipData GenerateMipTexels3D(const ermy::u8* srcTexelData, ermy::u32 width, ermy::u32 height, ermy::u32 depth, ermy::rendering::Format format);

	void RegenerateMips(CMP_MipSet& mipSet, int minSize = 1);

	CMP_ERROR CompressMips(CMP_MipSet& srcSet, 
						  CMP_MipSet& dstSet, 
						  ermy::rendering::Format sourceFormat, 
						  ermy::rendering::Format targetFormat,
						  const CompressionSettings& settings = CompressionSettings());

	CMP_ERROR CreateBlockEncoder(void** encoder, 
							   ermy::rendering::Format format,
							   const CompressionSettings& settings = CompressionSettings());
	CMP_ERROR CompressBlock(void** encoder,
						   void* srcBlock,
						   unsigned int srcStride,
						   void* dstBlock,
						   unsigned int dstStride);
	void DestroyBlockEncoder(void** encoder);

	CMP_ERROR AnalyzeTexture(CMP_MipSet& src1,
							CMP_MipSet& src2,
							CMP_AnalysisData& analysis);

	// New methods for array textures
	CMP_ERROR LoadArray(const char* utf8_file, CMP_MipSet& mipSet, int arraySize);
	CMP_ERROR SaveArray(const char* utf8_file, const CMP_MipSet& mipSet);
	
	void RegenerateMipsForArray(CMP_MipSet& mipSet, int minSize = 1);

private:
	CMP_ERROR SetupCompressionOptions(KernelOptions& options,
									const CompressionSettings& settings,
									ermy::rendering::Format targetFormat);
	CMP_ERROR SetupBC7Options(void* bc7Options,
							 const CompressionSettings& settings);

	// Helper for array texture setup
	CMP_ERROR InitializeArrayMipSet(CMP_MipSet& mipSet, 
								   int width, 
								   int height, 
								   int arraySize,
								   ChannelFormat channelFormat,
								   CMP_FORMAT format);
};
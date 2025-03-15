#include "compressonator_lib.h"
using namespace ermy::rendering;

CompressonatorLib::CompressonatorLib()
{
	CMP_InitFramework();
	CMP_InitializeBCLibrary();
}

CompressonatorLib::~CompressonatorLib()
{
}


CompressonatorLib& CompressonatorLib::Instance()
{
	static CompressonatorLib instance;
	return instance;
}

CMP_MipSet CompressonatorLib::Load(const char* utf8_file)
{
	CMP_MipSet result = {};
	auto errCode= CMP_LoadTexture(utf8_file, &result);
	return result;
}

void CompressonatorLib::RegenerateMips(CMP_MipSet& mipSet)
{
    CMP_GenerateMIPLevels(&mipSet, 1);

    printf("foo");
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
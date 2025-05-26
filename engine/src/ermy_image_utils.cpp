#include "ermy_utils.h"
#include <array>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


void ermy_utils::image::SaveImage(const std::filesystem::path& path, const ermy::u8* data, ermy::u32 width, ermy::u32 height, ermy::u32 channels)
{
    stbi_write_png(path.string().c_str(), width, height, channels, data, width * channels);
}

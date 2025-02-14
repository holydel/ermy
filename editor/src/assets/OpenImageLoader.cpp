#pragma once
#include "assets/OpenImageLoader.h"

#pragma warning(disable : 4244) // Disable C4244
#pragma warning(disable : 4267) // Disable C4267
#include <OpenImageIO/imageio.h>
#pragma warning(default : 4244) // Re-enable C4244
#pragma warning(default : 4267) // Re-enable C4267


#include <ermy_log.h>
#include <ermy_utils.h>
#include <assets/texture_asset.h>

using namespace OIIO;
using namespace editor::asset::loader;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& OpenImageLoader::SupportedExtensions()
{
    return gSupportedExtensions;
}

// Function to extract all extensions from the input string
static std::vector<std::string> extractExtensions(const std::string& input) {
    std::vector<std::string> extensions;

    // Step 1: Split the input string by ';' to get format blocks
    std::vector<std::string> formatBlocks = ermy_utils::string::split(input, ';');

    for (const std::string& block : formatBlocks) {
        // Step 2: Split each block by ':' to separate format name and extensions
        std::vector<std::string> parts = ermy_utils::string::split(block, ':');
        if (parts.size() == 2) { // Ensure the block has a valid format
            // Step 3: Split the extensions part by ',' to get individual extensions
            std::vector<std::string> extList = ermy_utils::string::split(parts[1], ',');
            // Step 4: Add all extensions to the result vector
            extensions.insert(extensions.end(), extList.begin(), extList.end());
        }
    }

    return extensions;
}

OpenImageLoader::OpenImageLoader()
{
	std::string supported_formats;
    getattribute("extension_list", supported_formats);
    ERMY_LOG("Supported image formats: %s\n", supported_formats.c_str());
    std::cout << "Supported image formats: " << supported_formats << std::endl;

    
    gSupportedExtensions = extractExtensions(supported_formats);
}

OpenImageLoader::~OpenImageLoader()
{

}

template <typename T>
void ConvertRGBtoRGBA(T* rgbDataOut, const T* rgbDataIn, int width, int height)
{
    for (int row = 0; row < height; ++row)
    {
        for (int x = 0; x < width; ++x)
        {
            int indexIn = (row * width + x) * 3;
            int indexOut = (row * width + x) * 4;

            rgbDataOut[indexOut + 0] = rgbDataIn[indexIn + 0];
            rgbDataOut[indexOut + 1] = rgbDataIn[indexIn + 1];
            rgbDataOut[indexOut + 2] = rgbDataIn[indexIn + 2];
            rgbDataOut[indexOut + 3] = static_cast<T>(0xFFFFFFFFu);
        }
    }
}

AssetData* OpenImageLoader::Load(const std::filesystem::path& path)
{
  //   Create an ImageInput object to read the image
   auto in = ImageInput::open(path.c_str());

   // Get the image specifications
   const ImageSpec& spec = in->spec();

   TextureAsset* result = new TextureAsset();

   result->width = spec.width;
   result->height = spec.height;
   result->numChannels = spec.nchannels;
   result->channelBytes = spec.channel_bytes();

   if (result->numChannels == 3)
       result->numChannels = 4;

   result->dataSize = result->width * result->height * result->numChannels * result->channelBytes;
   result->data = malloc(result->dataSize);
   
   //direct loading for 1,2,4 channels
   if (result->numChannels == spec.nchannels)
   {
       in->read_image(0, 0, 0, result->numChannels, spec.format, result->data);
   }
   else
   {
       int tempDataSize = result->width * result->height * result->channelBytes * spec.nchannels;
       void* temp = malloc(result->dataSize);
       in->read_image(0, 0, 0, spec.nchannels, spec.format, temp);
       
       if (result->channelBytes == 1)
           ConvertRGBtoRGBA(static_cast<ermy::u8*>(result->data), static_cast<ermy::u8*>(temp), result->width, result->height);

       if (result->channelBytes == 2)
           ConvertRGBtoRGBA(static_cast<ermy::u16*>(result->data), static_cast<ermy::u16*>(temp), result->width, result->height);

       if (result->channelBytes == 4)
           ConvertRGBtoRGBA(static_cast<ermy::u32*>(result->data), static_cast<ermy::u32*>(temp), result->width, result->height);

       free(temp);
   }

   result->RegeneratePreview();


   in->close();

   return result;
}
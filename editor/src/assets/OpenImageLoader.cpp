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
    ERMY_LOG(u8"Supported image formats: %s\n", supported_formats.c_str());
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
   auto in = ImageInput::open((const char*)path.u8string().c_str());

   if (!in)
   {
       in =ImageInput::open(path.wstring());
   }

   if (!in)
       return nullptr;

   // Get the image specifications
   const ImageSpec& spec = in->spec();

   TextureAsset* result = new TextureAsset();

   result->width = spec.width;
   result->height = spec.height;
   int numChannels = spec.nchannels;
   int channelBytes = spec.channel_bytes();

   if (numChannels == 3)
       numChannels = 4;

   auto dataSize = result->width * result->height * numChannels * channelBytes;
    
   void* data = malloc(dataSize);
   
   //TODO: texFormat evristics

   if (spec.channel_bytes() == 2)
   {
       result->texelSourceFormat = result->texelTargetFormat = ermy::rendering::Format::RGBA16F;
   }
   //direct loading for 1,2,4 channels
   if (numChannels == spec.nchannels)
   {
       in->read_image(0, 0, 0, numChannels, spec.format, data);       
   }
   else
   {
       int tempDataSize = result->width * result->height * channelBytes * spec.nchannels;
       void* temp = malloc(dataSize);
       in->read_image(0, 0, 0, spec.nchannels, spec.format, temp);
       
       if (channelBytes == 1)
           ConvertRGBtoRGBA(static_cast<ermy::u8*>(data), static_cast<ermy::u8*>(temp), result->width, result->height);

       if (channelBytes == 2)
           ConvertRGBtoRGBA(static_cast<ermy::u16*>(data), static_cast<ermy::u16*>(temp), result->width, result->height);

       if (channelBytes == 4)
           ConvertRGBtoRGBA(static_cast<ermy::u32*>(data), static_cast<ermy::u32*>(temp), result->width, result->height);

       free(temp);
   }

   result->SetSourceData(static_cast<ermy::u8*>(data), dataSize);

   in->close();

   return result;
}
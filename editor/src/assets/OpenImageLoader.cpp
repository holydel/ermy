#pragma once
#include "assets/OpenImageLoader.h"

#pragma warning(disable : 4244) // Disable C4244
#pragma warning(disable : 4267) // Disable C4267
#include <OpenImageIO/imageio.h>
#pragma warning(default : 4244) // Re-enable C4244
#pragma warning(default : 4267) // Re-enable C4267


#include <ermy_log.h>
#include <ermy_utils.h>


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

Asset* OpenImageLoader::Load(const std::filesystem::path& path)
{
    // Create an ImageInput object to read the image
  // auto in = ImageInput::open(filepath);

  // // Get the image specifications
  // const ImageSpec& spec = in->spec();
  // int width = spec.width;
  // int height = spec.height;
  // int channels = spec.nchannels;
  // int channelBytes = spec.channel_bytes();

  // std::cout << "Image dimensions: " << width << "x" << height << std::endl;
  // std::cout << "Number of channels: " << channels << std::endl;

  // // Allocate a buffer to hold the image data
  // std::vector<unsigned char> pixels(width * height * channels);


  // // Read the image data into the buffer
  //// in->read_image(TypeDesc::UINT8, &pixels[0]);
  // in->read_image(0, 0, 0, channels, spec.format, &pixels[0]);

  // // Close the image file
  // in->close();

	return nullptr;
}
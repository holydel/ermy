#include <assets/texture_asset.h>
#include <OpenImageIO/imageio.h>
#include <ermy_log.h>
#include <compressonator.h>
using namespace OIIO;

TextureAsset::TextureAsset()
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
}

TextureAsset::~TextureAsset()
{

}

//std::vector<std::string>  TextureAsset::Initialize()
//{
//    std::string supported_formats;
//    getattribute("input_format_list", supported_formats);
//    ERMY_LOG("Supported image formats: %s\n", supported_formats.c_str());
//    std::cout << "Supported image formats: " << supported_formats << std::endl;
//
//    std::vector<std::string> result;
//    return result;
//}
//
//void TextureAsset::Shutdown()
//{
//
//}
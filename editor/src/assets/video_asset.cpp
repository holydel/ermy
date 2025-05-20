#include <assets/video_asset.h>
#include <iostream>
#include <ermy_log.h>

extern "C" {
#include <libavformat/avformat.h>
}

VideoTextureAsset::VideoTextureAsset()
{

}

VideoTextureAsset::~VideoTextureAsset()
{

}

//std::vector<FormatExtensionInfo>  VideoTextureAsset::Initialize()
//{
//	//av_log_set_level(AV_LOG_QUIET);
//	avformat_network_init();
//
//    const AVInputFormat* input_format = nullptr;
//    void* opaque = nullptr; // Iterator state for av_demuxer_iterate()
//
//    std::cout << "Supported Input Formats:\n";
//    std::vector<FormatExtensionInfo> result;
//
//    while ((input_format = av_demuxer_iterate(&opaque))) {
//        if (input_format->name) {
//            ERMY_LOG(u8"support video format: %s\n", input_format->name);
//
//            if (input_format->extensions) {
//                ERMY_LOG(u8"  video format extensions: %s\n", input_format->extensions);
//
//                //AVFormatContext* fmt_ctx = nullptr;
//                //int errXode = avformat_alloc_context(&fmt_ctx, "", input_format, nullptr);
//
//                //int video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
//                //int audio_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
//                //FormatExtensionInfo& finfo = result.emplace_back();
//                //finfo.ext = input_format->extensions;
//                //finfo.atype = AssetType::VideoTexture;
//               /* if (video_stream)
//                {
//                   finfo.atype = AssetType::VideoTexture;
//
//                }
//                else if (audio_stream)
//                {
//                    finfo.atype = AssetType::Sound;
//                }
//                else
//                {
//                    finfo.atype = AssetType::Texture;
//                }*/
//
//                //td::cout << " (Extensions: " << input_format->extensions << ")";
//            }
//           // std::cout << "\n";
//        }
//    }
//
//    {
//        FormatExtensionInfo& finfo = result.emplace_back();
//        finfo.ext = "mp4";
//        finfo.atype = AssetType::VideoTexture;
//    }
//
//    result.emplace_back(FormatExtensionInfo{ "mp4" ,AssetType::VideoTexture });    
//    result.emplace_back(FormatExtensionInfo{ "mkv" ,AssetType::VideoTexture });
//    result.emplace_back(FormatExtensionInfo{ "mov" ,AssetType::VideoTexture });
//    result.emplace_back(FormatExtensionInfo{ "webm" ,AssetType::VideoTexture });
//
//    return result;
//}
//
//void VideoTextureAsset::Shutdown()
//{
//	avformat_network_deinit();
//}
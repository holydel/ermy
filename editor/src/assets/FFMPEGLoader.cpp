#pragma once
#include "assets/FFMpegLoader.h"
#include <iostream>
#include <ermy_log.h>
#include <ermy_utils.h>
#include "assets/video_asset.h"
#include "assets/texture_asset.h"
#include "assets/sound_asset.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
}

using namespace editor::asset::loader;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& FFMpegLoader::SupportedExtensions()
{
    return gSupportedExtensions;
}

FFMpegLoader::FFMpegLoader()
{
	av_log_set_level(AV_LOG_QUIET);    
	avformat_network_init();

    std::vector<const AVCodec*> codecs;
    
    void* opaque = nullptr;
    //while (auto codec = av_codec_iterate(&opaque))
    //{
    //    codecs.push_back(codec);
    //}

    const AVInputFormat* input_format = nullptr;
    opaque = nullptr; // Iterator state for av_demuxer_iterate()

    std::cout << "Supported Input Formats:\n";

    while ((input_format = av_demuxer_iterate(&opaque))) {
        if (input_format->name) {
            ERMY_LOG("support video format: %s\n", input_format->name);

            if (input_format->extensions) {
                ERMY_LOG("  video format extensions: %s\n", input_format->extensions);

                auto vec = ermy_utils::string::split(input_format->extensions,',');
                gSupportedExtensions.insert(gSupportedExtensions.end(), vec.begin(), vec.end());
            }
        }
    }

    int a = 52;
}

FFMpegLoader::~FFMpegLoader()
{
    avformat_network_deinit();
}

AssetData* FFMpegLoader::Load(const std::filesystem::path& path)
{
    AVFormatContext* format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, path.string().c_str(), nullptr, nullptr) != 0) {
        std::cerr << "(FFMPEG) Failed to open file: " << path << std::endl;
        return nullptr;
    }
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        std::cerr << "(FFMPEG) Failed to find stream information for file: " << path << std::endl;
        avformat_close_input(&format_ctx);
        return nullptr;
    }
    
    AssetData* result = nullptr;

    if (format_ctx->duration == AV_NOPTS_VALUE) {
        result = new TextureAsset(); // Assume it's a single-frame asset (e.g., an image)
    }
    else {
        // Check for video and audio streams
        int video_stream_index = -1;
        int audio_stream_index = -1;

        for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
            AVStream* stream = format_ctx->streams[i];
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_stream_index = i;
            }
            else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audio_stream_index = i;
            }
        }

        // Create the appropriate asset based on the streams
        if (video_stream_index != -1) {
            result = new VideoTextureAsset(); // Video asset
        }
        else if (audio_stream_index != -1) {
            SoundAsset* snd = new SoundAsset(); // Audio asset
            double duration_seconds = static_cast<double>(format_ctx->duration) / AV_TIME_BASE; 
            snd->duration = duration_seconds;

            result = snd;
        }
        else {
            std::cerr << "(FFMPEG) No video or audio streams found in file: " << path << std::endl;
        }
    }

    // Clean up
    avformat_close_input(&format_ctx);
    

	return result;
}
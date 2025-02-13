#pragma once
#include "assets/FFMpegLoader.h"
#include <iostream>
#include <ermy_log.h>
#include <ermy_utils.h>

extern "C" {
#include <libavformat/avformat.h>
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

Asset* FFMpegLoader::Load(const std::filesystem::path& path)
{
	return nullptr;
}
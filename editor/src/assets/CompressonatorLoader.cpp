#pragma once
#include "assets/CompressonatorLoader.h"
#include "compressonator_lib.h"

#include "assets/texture_asset.h"
#include <ermy_rendering.h>

using namespace editor::asset::loader;
using namespace ermy;
using namespace ermy::rendering;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& CompressonatorLoader::SupportedExtensions()
{
	gSupportedExtensions.push_back("dds");
	gSupportedExtensions.push_back("ktx");
	gSupportedExtensions.push_back("ktx2");
	return gSupportedExtensions;
}

CompressonatorLoader::CompressonatorLoader()
{

}

CompressonatorLoader::~CompressonatorLoader()
{

}



AssetData* CompressonatorLoader::Load(const std::filesystem::path& path)
{
	TextureAsset* result = new TextureAsset();

	result->GetSourceMipSet() = CompressonatorLib::Instance().Load(path.string().c_str());

	if (result->GetSourceMipSet().m_format == CMP_FORMAT_Unknown)
	{
		delete result;
		return nullptr;
	}

	auto& mip = result->GetSourceMipSet();

	result->width = mip.m_nWidth;
	result->height = mip.m_nHeight;
	result->texelSourceFormat = result->texelSourceFormat = CompressonatorLib::ConvertCMPFormatToFormat(mip.m_format);
	result->isSparse = false;
	result->depth = 1;
	result->numLayers = 1;

	if (result->texelSourceFormat == Format::UNKNOWN)
	{
		delete result;
		return nullptr;
	}

	//TT_2D = 0,  // A regular 2D texture. data stored linearly (rgba,rgba,...rgba)
	//TT_CubeMap = 1,  // A cubemap texture.
	//TT_VolumeTexture = 2,  // A volume texture.
	//TT_2D_Block = 3,  // 2D texture data stored as [Height][Width] blocks as individual channels using cmp_rgb_t or cmp_yuv_t
	//TT_1D = 4,  // Untyped data stored linearly
	if (mip.m_TextureType == TT_2D)
	{
		result->numLayers = mip.m_nDepth;
	}

	if (mip.m_TextureType == TT_CubeMap)
	{
		result->numLayers = mip.m_nDepth;
		result->isCubemap = true;
	}

	if (mip.m_TextureType == TT_VolumeTexture)
	{
		result->depth = mip.m_nDepth;
	}

	//result->isCubemap = mip.m_Flags
	result->RegeneratePreview();

	return result;
}
#pragma once
#include "assets/KTXLoader.h"

using namespace editor::asset::loader;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& KTXLoader::SupportedExtensions()
{
	return gSupportedExtensions;
}

KTXLoader::KTXLoader()
{

}

KTXLoader::~KTXLoader()
{

}

AssetData* KTXLoader::Load(const std::filesystem::path& path)
{
	return nullptr;
}
#pragma once
#include <compressonator.h>
#include <ermy_rendering.h>

class CompressonatorLib
{
	CompressonatorLib();
	~CompressonatorLib();
public:
	static CompressonatorLib& Instance();

	CMP_MipSet Load(const char* utf8_file);

	static CMP_FORMAT ConvertFormatToCMPFormat(ermy::rendering::Format format);
	static ermy::rendering::Format ConvertCMPFormatToFormat(CMP_FORMAT cmpFormat);

	void RegenerateMips(CMP_MipSet& mipSet);
	void CompressMips(CMP_MipSet& srcSet, CMP_MipSet& dstSet, ermy::rendering::Format sourceFormat, ermy::rendering::Format targetFormat);
};
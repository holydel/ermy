#pragma once
#include <ermy_api.h>

namespace ermy
{
	namespace pak
	{
		constexpr u32 PAK_MAGIC = 0x4B415045; // "EPAK"

		bool MountPak(const char* filePathUtf8);
	}
}
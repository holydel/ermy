#pragma once
#include "ermy_api.h"
#include <vector>
#include <string>

namespace ermy_utils
{	
	namespace string
	{
		std::vector<std::string> split(const std::string& input, char delimiter);
		std::string toLower(const std::string& input);
		std::string toUpper(const std::string& input);
		std::string humanReadableFileSize(ermy::u64 bytes);
	}

	namespace hash
	{
		ermy::u64 CalculateCRC64(const ermy::u8* data, const ermy::u32 size);
	}
}
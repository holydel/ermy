#pragma once
#include "ermy_api.h"
#include <vector>
#include <string>
#include <filesystem>
#include <vector>

namespace ermy_utils
{	
	namespace string
	{
		std::vector<std::string> split(const std::string& input, char delimiter);
		std::string toLower(const std::string& input);
		std::string toUpper(const std::string& input);
		std::string toLower(const std::u8string& input);
		std::string toUpper(const std::u8string& input);
		std::string humanReadableFileSize(ermy::u64 bytes);
		std::string humanReadableFileDate(const std::filesystem::file_time_type& time);

		std::vector<char32_t> getCodepoints(const std::u8string& str);
	}

	namespace hash
	{
		ermy::u64 CalculateCRC64(const ermy::u8* data, const ermy::u32 size);
	}

	namespace file
	{
		std::vector<ermy::u8> ReadFile(const std::filesystem::path& path);
		bool WriteFile(const std::filesystem::path& path, const std::vector<ermy::u8>& data);
		
	}

	namespace math
	{
		ermy::u64 alignUpPow2(ermy::u64 value);
		ermy::u64 alignUp(ermy::u64 value, ermy::u32 align);
	}
}
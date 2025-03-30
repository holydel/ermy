#include "ermy_utils.h"
#include <array>
#include <fstream>

std::vector<ermy::u8> ermy_utils::file::ReadFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return std::vector<ermy::u8>();
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<ermy::u8> data(size);
	file.read(reinterpret_cast<char*>(data.data()), size);

	return data;
}

bool ermy_utils::file::WriteFile(const std::filesystem::path& path, const std::vector<ermy::u8>& data)
{
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	file.write(reinterpret_cast<const char*>(data.data()), data.size());
	return file.good();
}
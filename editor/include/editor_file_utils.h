#pragma once
#include <filesystem>
#include <fstream>

bool copyFile(const std::filesystem::path& sourcePath, std::ofstream& destinationStream);

template <typename T>
void writeBinary(std::ofstream& to, const T& value)
{
    to.write((const char*)&value, sizeof(value));
}

template <typename T>
void writeVector(std::ofstream& to, const std::vector<T>& vec)
{
    writeBinary(to, vec.size());
    to.write((const char*)vec.data(), vec.size() * sizeof(T));
}

template <typename T>
T readBinary(std::ifstream& from)
{
	T value;
	from.read((char*)&value, sizeof(value));
	return value;
}

template <typename T>
std::vector<T> readVector(std::ifstream& from)
{
	size_t size = readBinary<size_t>(from);
	std::vector<T> vec(size);
	from.read((char*)vec.data(), size * sizeof(T));
	return vec;
}
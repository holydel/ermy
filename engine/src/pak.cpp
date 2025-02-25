#include "pak.h"
#include <cstdio>
#include <fstream>
using namespace ermy;
using namespace ermy::pak;

bool ermy::pak::MountPak(const char* filePathUtf8)
{
    std::ifstream f(filePathUtf8, std::ios::binary);

	if (!f.is_open())
	{
		return false;
	}

	u32 magic = 0;
	f >> magic;
	if (magic != PAK_MAGIC)
	{
		return false;
	}


	return true;
}
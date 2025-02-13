#pragma once
#include "ermy_api.h"
#include <vector>
#include <string>

namespace ermy_utils
{	
	namespace string
	{
		std::vector<std::string> split(const std::string& input, char delimiter);
	}
}
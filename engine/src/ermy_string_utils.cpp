#include <ermy_utils.h>
#include <sstream>

using namespace ermy_utils::string;

std::vector<std::string> ermy_utils::string::split(const std::string& input, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(input); // Create a stringstream from the input string
    std::string token;

    // Split the string by the delimiter
    while (std::getline(ss, token, delimiter)) {
        result.push_back(token); // Add each token to the vector
    }

    return result;
}
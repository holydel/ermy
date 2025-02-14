#include <ermy_utils.h>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

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

std::string ermy_utils::string::toLower(const std::string& input)
{
    std::string result = input; // Create a copy of the input string
    // Use std::transform to convert each character to lowercase
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}
std::string ermy_utils::string::toUpper(const std::string& input)
{
    std::string result = input; // Create a copy of the input string
    // Use std::transform to convert each character to lowercase
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string ermy_utils::string::humanReadableFileSize(ermy::u64 bytes)
{
        const char* suffixes[] = { "", "KB", "MB", "GB", "TB", "PB", "EB" }; // Suffixes for units
        int suffix_index = 0;

        double count = static_cast<double>(bytes);

        // Determine the appropriate unit
        while (count >= 1024 && suffix_index < 6) { // 6 is the index of "EB" (exabytes)
            count /= 1024;
            suffix_index++;
        }

        // Format the output
        std::ostringstream oss;
        if (suffix_index == 0) {
            oss << static_cast<uint64_t>(count) << " " << suffixes[suffix_index]; // No decimal for bytes
        }
        else if (count < 10) {
            oss << std::fixed << std::setprecision(2) << count << " " << suffixes[suffix_index]; // 2 decimal places for small numbers
        }
        else if (count < 100) {
            oss << std::fixed << std::setprecision(1) << count << " " << suffixes[suffix_index]; // 1 decimal place for medium numbers
        }
        else {
            oss << std::fixed << std::setprecision(0) << count << " " << suffixes[suffix_index]; // No decimal for large numbers
        }

        return oss.str();
}

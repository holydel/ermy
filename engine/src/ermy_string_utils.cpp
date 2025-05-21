#include <ermy_utils.h>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cuchar>    // For std::mbrtoc32
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

std::string ermy_utils::string::humanReadableFileDate(const std::filesystem::file_time_type& time)
{
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

    // Convert to time_t
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

    // Convert to tm structure for local time
    std::tm tm;
#ifdef _WIN32
    auto err = localtime_s(&tm, &tt); // Use localtime_s instead of localtime
#else
    int err = 0;
    tm = *localtime(&tt);
#endif
    char buffer[80] = {};

    if (err == 0)
    {
        // Format the time into a string
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    }
    
    return std::string(buffer);
}

std::vector<char32_t> ermy_utils::string::getCodepoints(const std::u8string& str)
{
    std::vector<char32_t> result;
    const char* ptr = reinterpret_cast<const char*>(str.data());
    const char* end = ptr + str.size();
    std::mbstate_t state = {}; // Initialize conversion state

    while (ptr < end) {
        char32_t cp;
        size_t rc = std::mbrtoc32(&cp, ptr, end - ptr, &state);
        if (rc == static_cast<size_t>(-1) || rc == static_cast<size_t>(-2)) {
            // Handle invalid or incomplete sequence
            result.push_back(0xFFFD); // Unicode replacement character
            ptr += 1; // Skip one byte to continue parsing
        }
        else {
            result.push_back(cp);
            ptr += rc; // Advance by the number of bytes consumed
        }
    }
    return result;
}
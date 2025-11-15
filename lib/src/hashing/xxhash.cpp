#include <prism/hashing/xxhash.h>
#include <xxhash.h> // Assuming xxhash library is available
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace prism {
namespace hashing {

std::string calculate_xxh3_hash(const std::vector<char>& data) {
    if (data.empty()) {
        return "";
    }
    XXH64_hash_t hash = XXH3_64bits(data.data(), data.size());
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

std::string calculate_xxh128_hash(const std::vector<char>& data) {
    if (data.empty()) {
        return "";
    }
    XXH128_hash_t hash = XXH3_128bits(data.data(), data.size());
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash.high64 << std::setw(16) << std::setfill('0') << hash.low64;
    return ss.str();
}

} 
} 

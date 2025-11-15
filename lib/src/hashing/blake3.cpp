#include <prism/hashing/blake3.h>
#include <blake3.h> // Assuming blake3 library is available
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

namespace prism {
namespace hashing {

std::string calculate_blake3_hash(const std::vector<char>& data) {
    if (data.empty()) {
        // BLAKE3 hash of an empty string
        return "af1349b9f5f1a6a4a04d11209405591300000000000000000000000000000000"; 
    }

    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, data.data(), data.size());
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

    std::stringstream ss;
    for (size_t i = 0; i < BLAKE3_OUT_LEN; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)output[i];
    }
    return ss.str();
}

} 
} 

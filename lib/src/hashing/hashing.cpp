#include <prism/hashing.h>
#include <prism/core/logging.h>
#include <prism/hashing/openssl_hasher.h> // Will be refactored to internal functions
#include <prism/hashing/xxhash.h>
#include <prism/hashing/crc.h>
#include <prism/hashing/blake3.h>
#include <stdexcept>
#include <fstream>

namespace prism {
namespace hashing {

// Forward declarations for internal OpenSSL hash functions (will be defined in openssl_hasher.cpp)
namespace internal {
    std::string calculate_openssl_hash(const std::string& file_path, prism::core::HashType hash_type);
    std::string calculate_openssl_hash_from_data(const std::vector<char>& data, prism::core::HashType hash_type);
}

std::string calculate_hash(const std::string& file_path, prism::core::HashType hash_type) {
    if (hash_type == prism::core::HashType::NONE) {
        return "";
    }

    // Read file content into a vector<char>
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        core::log("Warning: File not found for hash calculation: '" + file_path + "'", core::LOG_WARN);
        return "";
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> data(size);
    if (!file.read(data.data(), size)) {
        core::log("Warning: Could not read file for hash calculation: '" + file_path + "'", core::LOG_WARN);
        return "";
    }

    return calculate_hash_from_data(data, hash_type);
}

std::string calculate_hash_from_data(const std::vector<char>& data, prism::core::HashType hash_type) {
    switch (hash_type) {
        case prism::core::HashType::NONE:
            return "";
        case prism::core::HashType::MD5:
        case prism::core::HashType::SHA1:
        case prism::core::HashType::SHA256:
        case prism::core::HashType::SHA384:
        case prism::core::HashType::SHA512:
        case prism::core::HashType::BLAKE2B:
        case prism::core::HashType::BLAKE2S:
        case prism::core::HashType::SHA3_256:
        case prism::core::HashType::SHA3_512:
        case prism::core::HashType::RIPEMD160:
        case prism::core::HashType::WHIRLPOOL:
        case prism::core::HashType::SHA224:
        case prism::core::HashType::SHA3_224:
        case prism::core::HashType::SHA3_384:
            return internal::calculate_openssl_hash_from_data(data, hash_type);
        case prism::core::HashType::XXHASH3:
            return calculate_xxh3_hash(data);
        case prism::core::HashType::XXHASH128:
            return calculate_xxh128_hash(data);
        case prism::core::HashType::CRC32:
            return calculate_crc32_hash(data);
        case prism::core::HashType::CRC64:
            return calculate_crc64_hash(data);
        case prism::core::HashType::BLAKE3:
            return calculate_blake3_hash(data);
        default:
            core::log("Error: Unknown hash type requested.", core::LOG_ERROR);
            throw std::runtime_error("Unknown hash type");
    }
}

} 
} 

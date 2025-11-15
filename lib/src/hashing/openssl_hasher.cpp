#include <prism/hashing/openssl_hasher.h>
#include <prism/core/logging.h>
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace prism {
namespace hashing {
namespace internal { // Wrap in internal namespace

const EVP_MD* get_evp_md(core::HashType hash_type) {
    switch (hash_type) {
        case core::HashType::MD5: return EVP_md5();
        case core::HashType::SHA1: return EVP_sha1();
        case core::HashType::SHA256: return EVP_sha256();
        case core::HashType::SHA384: return EVP_sha384();
        case core::HashType::SHA512: return EVP_sha512();
        case core::HashType::BLAKE2B: return EVP_blake2b512();
        case core::HashType::BLAKE2S: return EVP_blake2s256();
        case core::HashType::SHA3_256: return EVP_sha3_256();
        case core::HashType::SHA3_512: return EVP_sha3_512();
        case core::HashType::RIPEMD160: return EVP_ripemd160();
        case core::HashType::SHA224: return EVP_sha224();
        case core::HashType::SHA3_224: return EVP_sha3_224();
        case core::HashType::SHA3_384: return EVP_sha3_384();
        default: return nullptr;
    }
}

// Renamed from calculate_hash_from_data
std::string calculate_openssl_hash_from_data(const std::vector<char>& data, core::HashType hash_type) {
    if (hash_type == core::HashType::NONE || data.empty()) return "";
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = get_evp_md(hash_type);
    
    if (!md) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    
    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

// This function is now only a wrapper for calculate_openssl_hash_from_data
std::string calculate_openssl_hash(const std::string& file_path, core::HashType hash_type) {
    if (hash_type == core::HashType::NONE) return "";
    
    core::log("Starting hash calculation for '" + file_path + "'...", core::LOG_VERBOSE);
    
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
    
    std::string hash_result = calculate_openssl_hash_from_data(data, hash_type);
    core::log("Finished hash calculation for '" + file_path + "'.", core::LOG_VERBOSE);
    return hash_result;
}


} // namespace internal
} // namespace hashing
} // namespace prism
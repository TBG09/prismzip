#ifndef PRISM_HASHING_OPENSSL_HASHER_H
#define PRISM_HASHING_OPENSSL_HASHER_H

#include <string>
#include <vector>
#include <prism/core/types.h>

namespace prism {
namespace hashing {
namespace internal {

// These functions are now internal and called by the main hashing dispatcher
std::string calculate_openssl_hash(const std::string& file_path, prism::core::HashType hash_type);
std::string calculate_openssl_hash_from_data(const std::vector<char>& data, prism::core::HashType hash_type);

} // namespace internal
} // namespace hashing
} // namespace prism

#endif 

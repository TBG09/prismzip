#ifndef PRISM_HASHING_OPENSSL_HASHER_H
#define PRISM_HASHING_OPENSSL_HASHER_H

#include <string>
#include <vector>
#include <prism/core/types.h>

namespace prism {
namespace hashing {

std::string calculate_hash(const std::string& file_path, prism::core::HashType hash_type);
std::string calculate_hash_from_data(const std::vector<char>& data, prism::core::HashType hash_type);

} // namespace hashing
} // namespace prism

#endif // PRISM_HASHING_OPENSSL_HASHER_H

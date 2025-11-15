#ifndef PRISM_HASHING_XXHASH_H
#define PRISM_HASHING_XXHASH_H

#include <string>
#include <vector>
#include <cstdint>

namespace prism {
namespace hashing {

std::string calculate_xxh3_hash(const std::vector<char>& data);
std::string calculate_xxh128_hash(const std::vector<char>& data);

} 
} 

#endif 

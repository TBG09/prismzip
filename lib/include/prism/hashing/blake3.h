#ifndef PRISM_HASHING_BLAKE3_H
#define PRISM_HASHING_BLAKE3_H

#include <string>
#include <vector>
#include <cstdint>

namespace prism {
namespace hashing {

std::string calculate_blake3_hash(const std::vector<char>& data);

} 
} 

#endif 

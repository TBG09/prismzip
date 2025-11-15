#ifndef PRISM_HASHING_CRC_H
#define PRISM_HASHING_CRC_H

#include <string>
#include <vector>
#include <cstdint>

namespace prism {
namespace hashing {

std::string calculate_crc32_hash(const std::vector<char>& data);
std::string calculate_crc64_hash(const std::vector<char>& data);

} 
} 

#endif 

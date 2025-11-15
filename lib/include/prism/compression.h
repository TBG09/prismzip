#ifndef PRISM_COMPRESSION_H
#define PRISM_COMPRESSION_H

#include <vector>
#include <prism/core/types.h>

namespace prism {
namespace compression {

std::vector<char> compress_data(const std::vector<char>& data, prism::core::CompressionType comp_type, int level);
std::vector<char> decompress_data(const std::vector<char>& data, prism::core::CompressionType comp_type, size_t original_size);

} 
} 

#endif 

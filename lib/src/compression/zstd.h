#ifndef PRISM_COMPRESSION_ZSTD_H
#define PRISM_COMPRESSION_ZSTD_H

#include <vector>

namespace prism {
namespace compression {

std::vector<char> zstd_compress(const std::vector<char>& data, int level);
std::vector<char> zstd_decompress(const std::vector<char>& data, size_t original_size);

} 
} 

#endif 

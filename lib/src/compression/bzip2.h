#ifndef PRISM_COMPRESSION_BZIP2_H
#define PRISM_COMPRESSION_BZIP2_H

#include <vector>

namespace prism {
namespace compression {

std::vector<char> bzip2_compress(const std::vector<char>& data, int level);
std::vector<char> bzip2_decompress(const std::vector<char>& data, size_t original_size);

} 
} 

#endif 

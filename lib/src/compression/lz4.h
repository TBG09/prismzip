#ifndef PRISM_COMPRESSION_LZ4_H
#define PRISM_COMPRESSION_LZ4_H

#include <vector>

namespace prism {
namespace compression {

std::vector<char> lz4_compress(const std::vector<char>& data, int level);
std::vector<char> lz4_decompress(const std::vector<char>& data, size_t original_size);

} // namespace compression
} // namespace prism

#endif // PRISM_COMPRESSION_LZ4_H

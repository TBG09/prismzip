#ifndef PRISM_COMPRESSION_ZLIB_H
#define PRISM_COMPRESSION_ZLIB_H

#include <vector>

namespace prism {
namespace compression {

std::vector<char> zlib_compress(const std::vector<char>& data, int level);
std::vector<char> zlib_decompress(const std::vector<char>& data, size_t original_size);

} // namespace compression
} // namespace prism

#endif // PRISM_COMPRESSION_ZLIB_H

#ifndef PRISM_COMPRESSION_SNAPPY_H
#define PRISM_COMPRESSION_SNAPPY_H

#include <vector>
#include <cstddef>

namespace prism {
namespace compression {

std::vector<char> snappy_compress(const std::vector<char>& data);
std::vector<char> snappy_decompress(const std::vector<char>& data, size_t original_size);

} // namespace compression
} // namespace prism

#endif // PRISM_COMPRESSION_SNAPPY_H

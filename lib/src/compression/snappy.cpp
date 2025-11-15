#include "snappy.h"
#include <snappy.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> snappy_compress(const std::vector<char>& data) {
    std::string input(data.begin(), data.end());
    std::string compressed_output;
    snappy::Compress(input.data(), input.size(), &compressed_output);
    return std::vector<char>(compressed_output.begin(), compressed_output.end());
}

std::vector<char> snappy_decompress(const std::vector<char>& data, size_t original_size) {
    std::string input(data.begin(), data.end());
    std::string uncompressed_output;
    if (!snappy::Uncompress(input.data(), input.size(), &uncompressed_output)) {
        throw std::runtime_error("Snappy decompression failed.");
    }
    if (uncompressed_output.size() != original_size) {
        throw std::runtime_error("Snappy decompressed size mismatch.");
    }
    return std::vector<char>(uncompressed_output.begin(), uncompressed_output.end());
}

} // namespace compression
} // namespace prism

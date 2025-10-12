#include "zstd.h"
#include <zstd.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> zstd_compress(const std::vector<char>& data, int level) {
    size_t max_compressed = ZSTD_compressBound(data.size());
    std::vector<char> result(max_compressed);
    
    size_t compressed_size = ZSTD_compress(result.data(), max_compressed,
                                           data.data(), data.size(), level);
    
    if (!ZSTD_isError(compressed_size)) {
        result.resize(compressed_size);
        return result;
    } else {
        throw std::runtime_error("Zstd compression failed");
    }
}

std::vector<char> zstd_decompress(const std::vector<char>& data, size_t original_size) {
    std::vector<char> result(original_size);
    
    size_t decompressed_size = ZSTD_decompress(result.data(), original_size,
                                               data.data(), data.size());
    
    if (ZSTD_isError(decompressed_size)) {
        throw std::runtime_error("Zstd decompression failed");
    }
    result.resize(decompressed_size);
    return result;
}

} // namespace compression
} // namespace prism

#include "lz4.h"
#include <lz4.h>
#include <lz4hc.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> lz4_compress(const std::vector<char>& data, int level) {
    int max_compressed = LZ4_compressBound(data.size());
    std::vector<char> result(max_compressed);
    
    int compressed_size;
    if (level >= 9) {
        compressed_size = LZ4_compress_HC(data.data(), result.data(), 
                                         data.size(), max_compressed, level);
    } else {
        compressed_size = LZ4_compress_default(data.data(), result.data(), 
                                               data.size(), max_compressed);
    }
    
    if (compressed_size > 0) {
        result.resize(compressed_size);
        return result;
    } else {
        throw std::runtime_error("LZ4 compression failed");
    }
}

std::vector<char> lz4_decompress(const std::vector<char>& data, size_t original_size) {
    std::vector<char> result(original_size);
    
    int decompressed_size = LZ4_decompress_safe(data.data(), result.data(),
                                                 data.size(), original_size);
    
    if (decompressed_size < 0) {
        throw std::runtime_error("LZ4 decompression failed");
    }
    result.resize(decompressed_size);
    return result;
}

} // namespace compression
} // namespace prism

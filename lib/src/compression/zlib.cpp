#include "zlib.h"
#include <zlib.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> zlib_compress(const std::vector<char>& data, int level) {
    uLongf compressed_size = compressBound(data.size());
    std::vector<char> result(compressed_size);
    
    int ret = compress2(reinterpret_cast<Bytef*>(result.data()), &compressed_size, 
                       reinterpret_cast<const Bytef*>(data.data()), data.size(), level);
    
    if (ret == Z_OK) {
        result.resize(compressed_size);
        return result;
    } else {
        throw std::runtime_error("Zlib compression failed");
    }
}

std::vector<char> zlib_decompress(const std::vector<char>& data, size_t original_size) {
    std::vector<char> result(original_size);
    uLongf uncompressed_size = original_size;
    
    int ret = uncompress(reinterpret_cast<Bytef*>(result.data()), &uncompressed_size,
                        reinterpret_cast<const Bytef*>(data.data()), data.size());
    
    if (ret != Z_OK) {
        throw std::runtime_error("Zlib decompression failed");
    }
    result.resize(uncompressed_size);
    return result;
}

} 
} 

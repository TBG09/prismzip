#include "bzip2.h"
#include <bzlib.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> bzip2_compress(const std::vector<char>& data, int level) {
    unsigned int compressed_size = data.size() * 1.01 + 600;
    std::vector<char> result(compressed_size);
    
    int ret = BZ2_bzBuffToBuffCompress(result.data(), &compressed_size,
                                      const_cast<char*>(data.data()), data.size(), 
                                      level, 0, 30);
    
    if (ret == BZ_OK) {
        result.resize(compressed_size);
        return result;
    } else {
        throw std::runtime_error("BZip2 compression failed");
    }
}

std::vector<char> bzip2_decompress(const std::vector<char>& data, size_t original_size) {
    std::vector<char> result(original_size);
    unsigned int uncompressed_size = original_size;
    
    int ret = BZ2_bzBuffToBuffDecompress(result.data(), &uncompressed_size,
                                        const_cast<char*>(data.data()), data.size(), 0, 0);
    
    if (ret != BZ_OK) {
        throw std::runtime_error("BZip2 decompression failed");
    }
    result.resize(uncompressed_size);
    return result;
}

} // namespace compression
} // namespace prism

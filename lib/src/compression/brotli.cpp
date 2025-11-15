#include "brotli.h"
#include <brotli/encode.h>
#include <brotli/decode.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> brotli_compress(const std::vector<char>& data, int level) {
    size_t max_compressed = BrotliEncoderMaxCompressedSize(data.size());
    std::vector<char> result(max_compressed);
    size_t compressed_size = max_compressed;
    
    int ret = BrotliEncoderCompress(level, BROTLI_DEFAULT_WINDOW, 
                                   BROTLI_DEFAULT_MODE,
                                   data.size(), reinterpret_cast<const uint8_t*>(data.data()),
                                   &compressed_size, reinterpret_cast<uint8_t*>(result.data()));
    
    if (ret) {
        result.resize(compressed_size);
        return result;
    } else {
        throw std::runtime_error("Brotli compression failed");
    }
}

std::vector<char> brotli_decompress(const std::vector<char>& data, size_t original_size) {
    std::vector<char> result(original_size);
    size_t decompressed_size = original_size;
    
    BrotliDecoderResult ret = BrotliDecoderDecompress(
        data.size(), reinterpret_cast<const uint8_t*>(data.data()),
        &decompressed_size, reinterpret_cast<uint8_t*>(result.data()));
    
    if (ret != BROTLI_DECODER_RESULT_SUCCESS) {
        throw std::runtime_error("Brotli decompression failed");
    }
    result.resize(decompressed_size);
    return result;
}

} 
} 

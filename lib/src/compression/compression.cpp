#include <prism/compression.h>
#include <prism/core/logging.h>
#include "zlib.h"
#include "bzip2.h"
#include "lzma.h"
#include "lzma2.h"
#include "lz4.h"
#include "zstd.h"
#include "brotli.h"
#include "snappy.h" 
#include "lzo.h"    
#include <stdexcept> 

namespace prism {
namespace compression {

std::vector<char> compress_data(const std::vector<char>& data, prism::core::CompressionType comp_type, int level) {
    switch (comp_type) {
        case prism::core::CompressionType::NONE:
            return data;
        case prism::core::CompressionType::ZLIB:
        case prism::core::CompressionType::GZIP: 
            return zlib_compress(data, level);
        case prism::core::CompressionType::BZIP2:
            return bzip2_compress(data, level);
        case prism::core::CompressionType::LZMA:
            return lzma_compress(data, level);
        case prism::core::CompressionType::LZ4:
            return lz4_compress(data, level);
        case prism::core::CompressionType::ZSTD:
            return zstd_compress(data, level);
        case prism::core::CompressionType::BROTLI:
            return brotli_compress(data, level);
        case prism::core::CompressionType::SNAPPY:
            return snappy_compress(data); 
        case prism::core::CompressionType::LZO:
            return lzo_compress(data);    
        case prism::core::CompressionType::LZMA2:
            return lzma2_compress(data, level);
        default:
            prism::core::log("Warning: Compression type not supported, storing uncompressed", prism::core::LOG_WARN);
            return data;
    }
}

std::vector<char> decompress_data(const std::vector<char>& data, prism::core::CompressionType comp_type, size_t original_size) {
    switch (comp_type) {
        case prism::core::CompressionType::NONE:
            return data;
        case prism::core::CompressionType::ZLIB:
        case prism::core::CompressionType::GZIP:
            return zlib_decompress(data, original_size);
        case prism::core::CompressionType::BZIP2:
            return bzip2_decompress(data, original_size);
        case prism::core::CompressionType::LZMA:
            return lzma_decompress(data, original_size);
        case prism::core::CompressionType::LZ4:
            return lz4_decompress(data, original_size);
        case prism::core::CompressionType::ZSTD:
            return zstd_decompress(data, original_size);
        case prism::core::CompressionType::BROTLI:
            return brotli_decompress(data, original_size);
        case prism::core::CompressionType::SNAPPY:
            return snappy_decompress(data, original_size); 
        case prism::core::CompressionType::LZO:
            return lzo_decompress(data, original_size);    
        case prism::core::CompressionType::LZMA2:
            return lzma2_decompress(data, original_size);
        default:
            prism::core::log("Warning: Decompression type not supported", prism::core::LOG_WARN);
            return data;
    }
}

} 
} 

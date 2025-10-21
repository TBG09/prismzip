#include <prism/compression.h>
#include <prism/core/logging.h>
#include "zlib.h"
#include "bzip2.h"
#include "lzma.h"
#include "lz4.h"
#include "zstd.h"
#include "brotli.h"
#include <stdexcept> // Required for std::runtime_error

namespace prism {
namespace compression {

std::vector<char> compress_data(const std::vector<char>& data, prism::core::CompressionType comp_type, int level) {
    switch (comp_type) {
        case prism::core::CompressionType::NONE:
            return data;
        case prism::core::CompressionType::ZLIB:
        case prism::core::CompressionType::GZIP: // Gzip uses zlib format internally for this app
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
            throw std::runtime_error("SNAPPY compression not yet implemented.");
        case prism::core::CompressionType::LZO:
            throw std::runtime_error("LZO compression not yet implemented.");
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
            throw std::runtime_error("SNAPPY decompression not yet implemented.");
        case prism::core::CompressionType::LZO:
            throw std::runtime_error("LZO decompression not yet implemented.");
        default:
            prism::core::log("Warning: Decompression type not supported", prism::core::LOG_WARN);
            return data;
    }
}

} // namespace compression
} // namespace prism

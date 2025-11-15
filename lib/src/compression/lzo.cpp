#include "lzo.h"
#include <lzo/lzo1x.h>
#include <stdexcept>
#include <memory>

namespace prism {
namespace compression {

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(lzo_compress_workmem, LZO1X_1_MEM_COMPRESS);

std::vector<char> lzo_compress(const std::vector<char>& data) {
    if (lzo_init() != LZO_E_OK) {
        throw std::runtime_error("LZO initialization failed.");
    }

    std::vector<char> compressed_data(data.size() + data.size() / 16 + 64 + 3); 
    lzo_uint out_len;

    int r = lzo1x_1_compress(reinterpret_cast<const lzo_bytep>(data.data()), data.size(),
                             reinterpret_cast<lzo_bytep>(compressed_data.data()), &out_len,
                             lzo_compress_workmem);

    if (r == LZO_E_OK) {
        compressed_data.resize(out_len);
        return compressed_data;
    } else {
        throw std::runtime_error("LZO compression failed with error code: " + std::to_string(r));
    }
}

std::vector<char> lzo_decompress(const std::vector<char>& data, size_t original_size) {
    if (lzo_init() != LZO_E_OK) {
        throw std::runtime_error("LZO initialization failed.");
    }

    std::vector<char> decompressed_data(original_size);
    lzo_uint out_len = original_size;

    int r = lzo1x_decompress_safe(reinterpret_cast<const lzo_bytep>(data.data()), data.size(),
                                 reinterpret_cast<lzo_bytep>(decompressed_data.data()), &out_len,
                                 NULL); 

    if (r == LZO_E_OK && out_len == original_size) {
        return decompressed_data;
    } else {
        throw std::runtime_error("LZO decompression failed with error code: " + std::to_string(r));
    }
}

} // namespace compression
} // namespace prism

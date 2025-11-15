#include "lzma.h"
#include <lzma.h>
#include <stdexcept>

namespace prism {
namespace compression {

std::vector<char> lzma_compress(const std::vector<char>& data, int level) {
    size_t compressed_size = data.size() * 1.5 + 1024;
    std::vector<char> result(compressed_size);
    
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_easy_encoder(&strm, level, LZMA_CHECK_CRC64);
    
    if (ret == LZMA_OK) {
        strm.next_in = reinterpret_cast<const uint8_t*>(data.data());
        strm.avail_in = data.size();
        strm.next_out = reinterpret_cast<uint8_t*>(result.data());
        strm.avail_out = result.size();
        
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret == LZMA_STREAM_END) {
            result.resize(strm.total_out);
            lzma_end(&strm);
            return result;
        }
        lzma_end(&strm);
    }
    
    throw std::runtime_error("LZMA compression failed");
}

std::vector<char> lzma_decompress(const std::vector<char>& data, size_t original_size) {
    std::vector<char> result(original_size);
    
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, 0);
    
    if (ret == LZMA_OK) {
        strm.next_in = reinterpret_cast<const uint8_t*>(data.data());
        strm.avail_in = data.size();
        strm.next_out = reinterpret_cast<uint8_t*>(result.data());
        strm.avail_out = result.size();
        
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret == LZMA_STREAM_END) {
            result.resize(strm.total_out);
            lzma_end(&strm);
            return result;
        }
        lzma_end(&strm);
    }
    
    throw std::runtime_error("LZMA decompression failed");
}

} 
} 

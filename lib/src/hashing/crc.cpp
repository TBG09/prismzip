#include <prism/hashing/crc.h>
#include <zlib.h> // For crc32
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace prism {
namespace hashing {

std::string calculate_crc32_hash(const std::vector<char>& data) {
    if (data.empty()) {
        return "00000000"; // Standard CRC32 for empty data
    }
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, reinterpret_cast<const Bytef*>(data.data()), data.size());
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << crc;
    return ss.str();
}

// Simple CRC64 implementation (can be optimized with lookup tables)
// Polynomial for CRC-64-ECMA: 0xC96C5795D7870F42
uint64_t crc64_ecma_update(uint64_t crc, const unsigned char *buf, size_t len) {
    static uint64_t crc_table[256];
    static bool table_initialized = false;

    if (!table_initialized) {
        uint64_t polynomial = 0xC96C5795D7870F42ULL;
        for (int i = 0; i < 256; i++) {
            uint64_t current_crc = i;
            for (int j = 0; j < 8; j++) {
                if (current_crc & 1) {
                    current_crc = (current_crc >> 1) ^ polynomial;
                } else {
                    current_crc >>= 1;
                }
            }
            crc_table[i] = current_crc;
        }
        table_initialized = true;
    }

    crc = ~crc; // Initial XOR
    for (size_t i = 0; i < len; i++) {
        crc = crc_table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
    }
    return ~crc; // Final XOR
}

std::string calculate_crc64_hash(const std::vector<char>& data) {
    if (data.empty()) {
        return "0000000000000000"; // Standard CRC64 for empty data
    }
    uint64_t crc = crc64_ecma_update(0ULL, reinterpret_cast<const unsigned char*>(data.data()), data.size());
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << crc;
    return ss.str();
}

} 
} 

#ifndef PRISM_CORE_TYPES_H
#define PRISM_CORE_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>

namespace prism {
namespace core {

enum class CompressionType : uint8_t { 
    NONE = 0, 
    ZLIB = 1, 
    BZIP2 = 2, 
    LZMA = 3, 
    GZIP = 4,
    LZ4 = 5,
    ZSTD = 6,
    BROTLI = 7,
    SNAPPY = 8,
    LZO = 9
};

enum class HashType : uint8_t { 
    NONE = 0, 
    MD5 = 1, 
    SHA1 = 2, 
    SHA256 = 3, 
    SHA512 = 4, 
    SHA384 = 5, 
    BLAKE2B = 6,
    BLAKE2S = 7,
    SHA3_256 = 8,
    SHA3_512 = 9,
    RIPEMD160 = 10,
    WHIRLPOOL = 11,
    SHA224 = 12,
    SHA3_224 = 13,
    SHA3_384 = 14
};

const uint8_t SOLID_ARCHIVE_FLAG = 0x01;
extern const char* SOLID_BLOCK_MAGIC;

struct FileMetadata {
    std::string path;
    uint64_t header_start_offset;
    uint64_t data_start_offset;
    CompressionType compression_type;
    uint8_t level;
    HashType hash_type;
    std::string file_hash;
    uint64_t file_size;
    uint64_t compressed_size;
    uint64_t creation_time;     
    uint64_t modification_time; 
    uint32_t permissions;       
    uint32_t uid;               
    uint32_t gid;               
};

extern const std::map<std::string, CompressionType> COMPRESSION_MAP;
extern const std::map<CompressionType, std::string> COMPRESSION_NAMES;
extern const std::map<std::string, HashType> HASH_MAP;
extern const std::map<HashType, std::string> HASH_NAMES;
extern const std::set<std::string> COMPRESSED_EXTENSIONS;

} 
} 

#endif
#ifndef PRISM_CORE_ARCHIVE_WRITER_H
#define PRISM_CORE_ARCHIVE_WRITER_H

#include <prism/core/types.h>
#include <prism/core/result_types.h>
#include <prism/core/logging.h> 
#include <cstring> 
#include <string>
#include <vector>
#include <cstdint>

namespace prism {
namespace core {

inline std::vector<char> create_archive_header(const std::string& archive_path, CompressionType compression_type,
                                   uint8_t level, HashType hash_type, const std::string& file_hash,
                                   uint64_t file_size, uint64_t compressed_size,
                                   uint64_t creation_time, uint64_t modification_time,
                                   uint32_t permissions, uint32_t uid, uint32_t gid) {
    log("Creating header for '" + archive_path + "'...", LOG_VERBOSE);
    
    std::vector<char> header;
    
    uint32_t path_len_val = archive_path.size();
    for (int i = 0; i < sizeof(uint32_t); ++i) {
        header.push_back((path_len_val >> (i * 8)) & 0xFF);
    }
    header.insert(header.end(), archive_path.begin(), archive_path.end());
    
    header.push_back(static_cast<uint8_t>(compression_type));
    header.push_back(level);
    header.push_back(static_cast<uint8_t>(hash_type));
    
    uint16_t hash_len_val = file_hash.size();
    for (int i = 0; i < sizeof(uint16_t); ++i) {
        header.push_back((hash_len_val >> (i * 8)) & 0xFF);
    }
    header.insert(header.end(), file_hash.begin(), file_hash.end());
    
    for (int i = 0; i < sizeof(uint64_t); ++i) {
        header.push_back((file_size >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < sizeof(uint64_t); ++i) {
        header.push_back((compressed_size >> (i * 8)) & 0xFF);
    }

    for (int i = 0; i < sizeof(uint64_t); ++i) {
        header.push_back((creation_time >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < sizeof(uint64_t); ++i) {
        header.push_back((modification_time >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < sizeof(uint32_t); ++i) {
        header.push_back((permissions >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < sizeof(uint32_t); ++i) {
        header.push_back((uid >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < sizeof(uint32_t); ++i) {
        header.push_back((gid >> (i * 8)) & 0xFF);
    }
    
    log("Header creation complete.", LOG_VERBOSE);
    return header;
}

ArchiveCreationResult create_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                   CompressionType comp_type, int level, HashType hash_type, 
                   bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes = false, int num_threads = 1, bool raw_output = false, bool use_basic_chars = false, bool solid_mode = false);

ArchiveCreationResult append_to_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                      CompressionType comp_type, int level, HashType hash_type, 
                      bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes = false, int num_threads = 1, bool raw_output = false, bool use_basic_chars = false, bool solid_mode = false);

} 
} 

#endif 

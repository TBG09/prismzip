#ifndef PRISM_CORE_ARCHIVE_WRITER_H
#define PRISM_CORE_ARCHIVE_WRITER_H

#include <prism/core/types.h>
#include <prism/core/result_types.h>
#include <prism/core/logging.h> // For log()
#include <string>
#include <vector>
#include <cstdint>

namespace prism {
namespace core {

inline std::vector<char> create_archive_header(const std::string& archive_path, CompressionType compression_type,
                                   uint8_t level, HashType hash_type, const std::string& file_hash,
                                   uint64_t file_size, uint64_t compressed_size) {
    log("Creating header for '" + archive_path + "'...", LOG_VERBOSE);
    
    std::vector<char> header;
    
    uint32_t path_len = archive_path.size();
    header.insert(header.end(), (char*)&path_len, (char*)&path_len + 4);
    header.insert(header.end(), archive_path.begin(), archive_path.end());
    
    header.push_back(static_cast<uint8_t>(compression_type));
    header.push_back(level);
    header.push_back(static_cast<uint8_t>(hash_type));
    
    uint16_t hash_len = file_hash.size();
    header.insert(header.end(), (char*)&hash_len, (char*)&hash_len + 2);
    header.insert(header.end(), file_hash.begin(), file_hash.end());
    
    header.insert(header.end(), (char*)&file_size, (char*)&file_size + 8);
    header.insert(header.end(), (char*)&compressed_size, (char*)&compressed_size + 8);
    
    log("Header creation complete.", LOG_VERBOSE);
    return header;
}

ArchiveCreationResult create_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                   CompressionType comp_type, int level, HashType hash_type, 
                   bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes = false, int num_threads = 1, bool raw_output = false, bool use_basic_chars = false, bool solid_mode = false);

ArchiveCreationResult append_to_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                      CompressionType comp_type, int level, HashType hash_type, 
                      bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes = false, int num_threads = 1, bool raw_output = false, bool use_basic_chars = false, bool solid_mode = false);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_WRITER_H

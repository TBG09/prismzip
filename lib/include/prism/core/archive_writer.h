#ifndef PRISM_CORE_ARCHIVE_WRITER_H
#define PRISM_CORE_ARCHIVE_WRITER_H

#include <prism/core/types.h>
#include <string>
#include <vector>

namespace prism {
namespace core {

void create_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                   CompressionType comp_type, int level, HashType hash_type, 
                   bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path);

void append_to_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                      CompressionType comp_type, int level, HashType hash_type, 
                      bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path);

std::vector<char> create_archive_header(const std::string& archive_path, CompressionType compression_type,
                                   uint8_t level, HashType hash_type, const std::string& file_hash,
                                   uint64_t file_size, uint64_t compressed_size);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_WRITER_H

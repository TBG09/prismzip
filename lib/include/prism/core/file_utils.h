#ifndef PRISM_CORE_FILE_UTILS_H
#define PRISM_CORE_FILE_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <prism/core/types.h>

namespace prism {
namespace core {

std::string format_size(uint64_t bytes);
bool file_exists(const std::string& path);
bool is_directory(const std::string& path);
std::string get_extension(const std::string& path);
std::string get_absolute_path(const std::string& path);
void list_files_recursive(const std::string& dir_path, std::vector<std::string>& files, const std::vector<std::string>& exclude_patterns);
bool match_pattern(const std::string& path, const std::string& pattern);
bool should_exclude(const std::string& path, const std::vector<std::string>& exclude_patterns);
bool should_compress(const std::string& file_path, CompressionType compression_type);

} // namespace core
} // namespace prism

#endif
#ifndef PRISM_CORE_ARCHIVE_EXTRACTOR_H
#define PRISM_CORE_ARCHIVE_EXTRACTOR_H

#include <string>
#include <vector>
#include <atomic> // For std::atomic
#include <mutex> // For std::mutex
#include <prism/core/types.h> // For FileMetadata
#include <prism/core/result_types.h>
#include <prism/core/logging.h> // For log()
#include <prism/core/ui_utils.h> // For show_progress_bar

namespace prism {
namespace core {

ArchiveExtractionResult extract_archive(const std::string& archive_file, const std::string& output_dir, 
                     const std::vector<std::string>& files_to_extract, bool no_overwrite, bool no_verify, int num_threads, bool raw_output, bool use_basic_chars, bool no_preserve_props);

void extract_non_solid_file(const std::string& archive_file, const FileMetadata& item, const std::string& output_dir, bool no_overwrite, bool no_verify, std::atomic<int>& files_extracted, std::atomic<int>& files_skipped, std::atomic<uint64_t>& bytes_extracted, std::atomic<int>& hash_mismatches, std::atomic<int>& hashes_checked, std::atomic<int>& progress_counter, size_t total_items_to_process, std::mutex& cout_mutex, bool raw_output, bool use_basic_chars, bool no_preserve_props);

void extract_solid_block(const std::string& archive_file, const std::vector<FileMetadata>& block_items, const std::string& output_dir, bool no_overwrite, bool no_verify, std::atomic<int>& files_extracted, std::atomic<int>& files_skipped, std::atomic<uint64_t>& bytes_extracted, std::atomic<int>& hash_mismatches, std::atomic<int>& hashes_checked, std::atomic<int>& progress_counter, size_t total_items_to_process, std::mutex& cout_mutex, bool raw_output, bool use_basic_chars, bool no_preserve_props);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_EXTRACTOR_H

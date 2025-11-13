#ifndef PRISM_CORE_ARCHIVE_VERIFIER_H
#define PRISM_CORE_ARCHIVE_VERIFIER_H

#include <string>
#include <vector>
#include <atomic> // For std::atomic
#include <mutex> // For std::mutex
#include <prism/core/types.h> // For FileMetadata
#include <prism/core/thread_pool.h> // For ThreadPool
#include <prism/core/ui_utils.h> // For show_progress_bar

namespace prism {
namespace core {

void verify_archive(const std::string& archive_file, bool raw_output = false, bool use_basic_chars = false);

void verify_non_solid_file(const std::string& archive_file, const FileMetadata& item, const std::string& temp_dir, std::atomic<int>& mismatches, std::atomic<int>& checked_files, std::atomic<int>& progress_counter, size_t total_items_to_process, bool raw_output, bool use_basic_chars);

void verify_solid_block(const std::string& archive_file, const std::vector<FileMetadata>& block_items, const std::string& temp_dir, std::atomic<int>& mismatches, std::atomic<int>& checked_files, std::atomic<int>& progress_counter, size_t total_items_to_process, bool raw_output, bool use_basic_chars);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_VERIFIER_H

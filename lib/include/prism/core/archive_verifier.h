#ifndef PRISM_CORE_ARCHIVE_VERIFIER_H
#define PRISM_CORE_ARCHIVE_VERIFIER_H

#include <string>
#include <vector>
#include <atomic> 
#include <mutex> 
#include <prism/core/types.h> 
#include <prism/core/thread_pool.h> 
#include <prism/core/ui_utils.h> 

namespace prism {
namespace core {

void verify_archive(const std::string& archive_file, bool raw_output = false, bool use_basic_chars = false);

void verify_non_solid_file(const std::string& archive_file, const FileMetadata& item, const std::string& temp_dir, std::atomic<int>& mismatches, std::atomic<int>& checked_files, std::atomic<int>& progress_counter, size_t total_items_to_process, bool raw_output, bool use_basic_chars, std::chrono::steady_clock::time_point start_time);

void verify_solid_block(const std::string& archive_file, const std::vector<FileMetadata>& block_items, const std::string& temp_dir, std::atomic<int>& mismatches, std::atomic<int>& checked_files, std::atomic<int>& progress_counter, size_t total_items_to_process, bool raw_output, bool use_basic_chars, std::chrono::steady_clock::time_point start_time);

} 
} 

#endif 

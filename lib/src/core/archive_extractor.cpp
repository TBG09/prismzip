#include <prism/core/archive_extractor.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/compression.h>
#include <prism/hashing/openssl_hasher.h>
#include <prism/core/thread_pool.h>
#include <prism/core/ui_utils.h>
#include <fstream>
#include <iostream>
#include <set>
#include <sys/stat.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <stdexcept>

namespace prism {
namespace core {

void extract_non_solid_file(const std::string& archive_file, const FileMetadata& item, const std::string& output_dir, bool no_overwrite, bool no_verify, std::atomic<int>& files_extracted, std::atomic<int>& files_skipped, std::atomic<uint64_t>& bytes_extracted, std::atomic<int>& hash_mismatches, std::atomic<int>& hashes_checked, std::atomic<int>& progress_counter, size_t total_items_to_process, std::mutex& cout_mutex, bool raw_output, bool use_basic_chars, bool no_preserve_props, std::chrono::steady_clock::time_point start_time) {
    std::string out_path = output_dir + "/" + item.path;

    if (no_overwrite && file_exists(out_path)) {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            log("Skipping existing file: '" + item.path + "'", LOG_VERBOSE);
        }
        files_skipped++;
        return;
    }
    
    size_t pos = out_path.find_last_of('/');
    if (pos != std::string::npos) {
        std::string dir = out_path.substr(0, pos);
        system(("mkdir -p \"" + dir + "\"" ).c_str());
    }
    
    std::vector<char> compressed(item.compressed_size);
    {
        std::ifstream in(archive_file, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Cannot open archive: " + archive_file);
        }
        in.seekg(item.data_start_offset);
        in.read(compressed.data(), item.compressed_size);
    }
    
    std::vector<char> decompressed = compression::decompress_data(compressed, 
                                                                  item.compression_type,
                                                                  item.file_size);
    
    std::ofstream out_file(out_path, std::ios::binary);
    if (!out_file) {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            log("Warning: Cannot create file: '" + out_path + "'", LOG_WARN);
        }
        return;
    }
    
    out_file.write(decompressed.data(), decompressed.size());
    out_file.close();

    if (!no_preserve_props) {
        set_file_properties(out_path, item);
    }
    
    files_extracted++;
    bytes_extracted += item.file_size;
    
    if (!no_verify && item.hash_type != HashType::NONE) {
        hashes_checked++;
        std::string calculated_hash = hashing::calculate_hash(out_path, item.hash_type);
        if (calculated_hash != item.file_hash) {
            hash_mismatches++;
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                log("Hash mismatch for '" + item.path + "'. Data may be corrupted.", LOG_WARN);
            }
        } else {
            std::lock_guard<std::mutex> lock(cout_mutex);
            log("Hash verified for '" + item.path + "'", LOG_VERBOSE);
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        int current_progress = ++progress_counter;
        show_progress_bar(current_progress, total_items_to_process, item.path, item.file_size, item.compressed_size, start_time, raw_output, use_basic_chars);
    }
}

void extract_solid_block(const std::string& archive_file, const std::vector<FileMetadata>& block_items, const std::string& output_dir, bool no_overwrite, bool no_verify, std::atomic<int>& files_extracted, std::atomic<int>& files_skipped, std::atomic<uint64_t>& bytes_extracted, std::atomic<int>& hash_mismatches, std::atomic<int>& hashes_checked, std::atomic<int>& progress_counter, size_t total_items_to_process, std::mutex& cout_mutex, bool raw_output, bool use_basic_chars, bool no_preserve_props, std::chrono::steady_clock::time_point start_time) {
    if (block_items.empty()) return;

    const FileMetadata& first_item = block_items[0];

    log("Debug: first_item.compressed_size = " + std::to_string(first_item.compressed_size), LOG_DEBUG);

    std::vector<char> compressed_block(first_item.compressed_size);
    {
        std::ifstream in(archive_file, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Cannot open archive: " + archive_file);
        }
        in.seekg(first_item.header_start_offset);
        in.read(compressed_block.data(), first_item.compressed_size);
    }

    uint64_t total_uncompressed_size_in_block = 0;
    for (const auto& item : block_items) {
        total_uncompressed_size_in_block += item.file_size;
    }
    log("Debug: total_uncompressed_size_in_block = " + std::to_string(total_uncompressed_size_in_block), LOG_DEBUG);
    log("Debug: compressed_block.size() = " + std::to_string(compressed_block.size()), LOG_DEBUG);

    std::vector<char> decompressed_block = compression::decompress_data(compressed_block,
                                                                      first_item.compression_type,
                                                                      total_uncompressed_size_in_block);
    log("Debug: decompressed_block.size() = " + std::to_string(decompressed_block.size()), LOG_DEBUG);

    for (const auto& item : block_items) {
        std::string out_path = output_dir + "/" + item.path;

        if (no_overwrite && file_exists(out_path)) {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                log("Skipping existing file: '" + item.path + "'", LOG_VERBOSE);
            }
            files_skipped++;
            continue;
        }

        size_t pos = out_path.find_last_of('/');
        if (pos != std::string::npos) {
            std::string dir = out_path.substr(0, pos);
            system(("mkdir -p \"" + dir + "\"" ).c_str());
        }

        std::vector<char> file_data(decompressed_block.begin() + item.data_start_offset,
                                    decompressed_block.begin() + item.data_start_offset + item.file_size);

        std::ofstream out_file(out_path, std::ios::binary);
        if (!out_file) {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                log("Warning: Cannot create file: '" + out_path + "'", LOG_WARN);
            }
            continue;
        }

        out_file.write(file_data.data(), file_data.size());
        out_file.close();

        if (!no_preserve_props) {
            set_file_properties(out_path, item);
        }

        files_extracted++;
        bytes_extracted += item.file_size;

        if (!no_verify && item.hash_type != HashType::NONE) {
            hashes_checked++;
            std::string calculated_hash = hashing::calculate_hash(out_path, item.hash_type);
            if (calculated_hash != item.file_hash) {
                hash_mismatches++;
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    log("Hash mismatch for '" + item.path + "'. Data may be corrupted.", LOG_WARN);
                }
            } else {
                std::lock_guard<std::mutex> lock(cout_mutex);
                log("Hash verified for '" + item.path + "'", LOG_VERBOSE);
            }
        }

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            int current_progress = ++progress_counter;
            show_progress_bar(current_progress, total_items_to_process, item.path, item.file_size, item.compressed_size, start_time, raw_output, use_basic_chars);
        }
    }
}

ArchiveExtractionResult extract_archive(const std::string& archive_file, const std::string& output_dir, 
                     const std::vector<std::string>& files_to_extract, bool no_overwrite, bool no_verify, int num_threads, bool raw_output, bool use_basic_chars, bool no_preserve_props) {
    auto start_time = std::chrono::steady_clock::now();
    std::vector<FileMetadata> items = read_archive_metadata(archive_file);
    
    std::vector<FileMetadata> items_to_process;
    if (!files_to_extract.empty()) {
        std::set<std::string> requested_files;
        std::vector<std::string> requested_dirs;

        for (const auto& path : files_to_extract) {
            if (path.back() == '/' || path.back() == '\\') {
                requested_dirs.push_back(path);
            } else {
                requested_files.insert(path);
            }
        }

        for (const auto& item : items) {
            if (requested_files.count(item.path)) {
                items_to_process.push_back(item);
                continue;
            }
            for (const auto& dir : requested_dirs) {
                if (item.path.rfind(dir, 0) == 0) {
                    items_to_process.push_back(item);
                    break; 
                }
                if (item.path.size() > dir.size() && item.path.rfind(dir, 0) == 0 && item.path[dir.size()] == '/') {
                    items_to_process.push_back(item);
                    break;
                }
            }
        }
        
        if (items_to_process.empty()) {
            log("Warning: No matching files found in archive for the given paths", LOG_WARN);
            return {0, 0, 0, 0, 0, {}};
        }
    } else {
        items_to_process = items;
    }
    
    std::atomic<int> files_extracted = 0;
    std::atomic<int> files_skipped = 0;
    std::atomic<uint64_t> bytes_extracted = 0;
    std::atomic<int> hash_mismatches = 0;
    std::atomic<int> hashes_checked = 0;
    std::atomic<int> progress_counter = 0;
    
    std::mutex cout_mutex;
    std::vector<long long> durations_ms;

    std::map<uint64_t, std::vector<FileMetadata>> solid_blocks;
    std::vector<FileMetadata> non_solid_files;

    for (const auto& item : items_to_process) {
        if (!item.is_solid) {
            non_solid_files.push_back(item);
        } else {
            solid_blocks[item.header_start_offset].push_back(item);
        }
    }

    {
        ThreadPool pool(num_threads);
        std::vector<std::future<void>> results;

        for (const auto& item : non_solid_files) {
            results.emplace_back(pool.enqueue([&, item] {
                extract_non_solid_file(archive_file, item, output_dir, no_overwrite, no_verify, files_extracted, files_skipped, bytes_extracted, hash_mismatches, hashes_checked, progress_counter, items_to_process.size(), cout_mutex, raw_output, use_basic_chars, no_preserve_props, start_time);
            }));
        }

        for (const auto& pair : solid_blocks) {
            results.emplace_back(pool.enqueue([&, pair] {
                extract_solid_block(archive_file, pair.second, output_dir, no_overwrite, no_verify, files_extracted, files_skipped, bytes_extracted, hash_mismatches, hashes_checked, progress_counter, items_to_process.size(), cout_mutex, raw_output, use_basic_chars, no_preserve_props, start_time);
            }));
        }

        for(auto && result : results)
            result.get();
        
        durations_ms = pool.get_thread_durations();
    }
    
    if (items_to_process.size() > 0 && !raw_output) std::cout << std::endl;
    
    log("Successfully extracted from archive '" + archive_file + "'", LOG_SUCCESS);
    log("Files extracted: " + std::to_string(files_extracted.load()), LOG_SUM);
    if (files_skipped > 0) {
        log("Files skipped (already exist): " + std::to_string(files_skipped.load()), LOG_SUM);
    }
    log("Total data extracted: " + format_size(bytes_extracted.load()), LOG_SUM);
    
    if (!no_verify) {
        if (hashes_checked > 0) {
            if (hash_mismatches == 0) {
                log("Integrity Check: All hashes matched. File integrity verified.", LOG_SUM);
            } else {
                log("Integrity Check: " + std::to_string(hash_mismatches.load()) + " hash mismatches found.", LOG_WARN);
            }
        }
    } else {
        log("Integrity Check: Skipped.", LOG_SUM);
    }

    return {files_extracted.load(), files_skipped.load(), bytes_extracted.load(), hashes_checked.load(), hash_mismatches.load(), durations_ms};
}

} // namespace core
} // namespace prism
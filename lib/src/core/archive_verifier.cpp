#include <prism/core/archive_verifier.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/compression.h>
#include <prism/hashing/openssl_hasher.h>
#include <prism/core/ui_utils.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

namespace prism {
namespace core {

void verify_non_solid_file(const std::string& archive_file, const FileMetadata& item, const std::string& temp_dir, std::atomic<int>& mismatches, std::atomic<int>& checked_files, std::atomic<int>& progress_counter, size_t total_items_to_process, bool raw_output, bool use_basic_chars) {
    if (item.hash_type == HashType::NONE) {
        return;
    }

    std::string out_path = temp_dir + "/" + item.path + "_" + std::to_string(progress_counter.load());
    
    std::vector<char> compressed_data(item.compressed_size);
    std::ifstream in(archive_file, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open archive for reading: " + archive_file);
    }
    in.seekg(item.data_start_offset);
    in.read(compressed_data.data(), item.compressed_size);
    in.close();

    std::vector<char> decompressed_data = compression::decompress_data(compressed_data, item.compression_type, item.file_size);
    
    std::ofstream out_file(out_path, std::ios::binary);
    if (!out_file) {
        log("Could not write temporary file for verification: " + out_path, LOG_WARN);
        return;
    }
    out_file.write(decompressed_data.data(), decompressed_data.size());
    out_file.close();

    std::string calculated_hash = hashing::calculate_hash(out_path, item.hash_type);
    checked_files++;

    if (calculated_hash != item.file_hash) {
        mismatches++;
        log("Hash mismatch for: '" + item.path + "'", LOG_WARN);
        log("  - Expected: " + item.file_hash, LOG_WARN);
        log("  - Got:      " + calculated_hash, LOG_WARN);
    }
    
    show_progress_bar(progress_counter.fetch_add(1) + 1, total_items_to_process, item.path, item.file_size, raw_output, use_basic_chars);
}

void verify_solid_block(const std::string& archive_file, const std::vector<FileMetadata>& block_items, const std::string& temp_dir, std::atomic<int>& mismatches, std::atomic<int>& checked_files, std::atomic<int>& progress_counter, size_t total_items_to_process, bool raw_output, bool use_basic_chars) {
    if (block_items.empty()) return;

    const FileMetadata& first_item = block_items[0];

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

    std::vector<char> decompressed_block = compression::decompress_data(compressed_block,
                                                                      first_item.compression_type,
                                                                      total_uncompressed_size_in_block);

    for (const auto& item : block_items) {
        if (item.hash_type == HashType::NONE) {
            continue;
        }

        std::string out_path = temp_dir + "/" + item.path + "_" + std::to_string(progress_counter.load());

        std::vector<char> file_data(decompressed_block.begin() + item.data_start_offset,
                                    decompressed_block.begin() + item.data_start_offset + item.file_size);

        std::ofstream out_file(out_path, std::ios::binary);
        if (!out_file) {
            log("Could not write temporary file for verification: " + out_path, LOG_WARN);
            continue;
        }
        out_file.write(file_data.data(), file_data.size());
        out_file.close();

        std::string calculated_hash = hashing::calculate_hash(out_path, item.hash_type);
        checked_files++;

        if (calculated_hash != item.file_hash) {
            mismatches++;
            log("Hash mismatch for: '" + item.path + "'", LOG_WARN);
            log("  - Expected: " + item.file_hash, LOG_WARN);
            log("  - Got:      " + calculated_hash, LOG_WARN);
        }
        
        show_progress_bar(progress_counter.fetch_add(1) + 1, total_items_to_process, item.path, item.file_size, raw_output, use_basic_chars);
    }
}

void verify_archive(const std::string& archive_file, bool raw_output, bool use_basic_chars) {
    log("Verifying archive: '" + archive_file + "'", LOG_INFO);

    std::vector<FileMetadata> items = read_archive_metadata(archive_file);
    if (items.empty()) {
        log("Archive is empty or metadata is corrupted.", LOG_WARN);
        return;
    }

    std::string temp_dir = "prism_verify_temp";
    if (fs::exists(temp_dir)) {
        fs::remove_all(temp_dir);
    }
    fs::create_directory(temp_dir);

    std::atomic<int> mismatches = 0;
    std::atomic<int> checked_files = 0;
    std::atomic<int> progress_counter = 0;

    std::map<uint64_t, std::vector<FileMetadata>> solid_blocks;
    std::vector<FileMetadata> non_solid_files;

    for (const auto& item : items) {
        if (item.hash_type == HashType::NONE) {
            continue;
        }

        if (item.header_start_offset == item.data_start_offset) {
            non_solid_files.push_back(item);
        } else {
            solid_blocks[item.header_start_offset].push_back(item);
        }
    }

    size_t total_items_to_process = non_solid_files.size();
    for (const auto& pair : solid_blocks) {
        total_items_to_process += pair.second.size();
    }

    if (total_items_to_process == 0) {
        log("Verification complete. No files had hashes to check.", LOG_SUM);
        fs::remove_all(temp_dir);
        return;
    }

    {
        ThreadPool pool(1);
        std::vector<std::future<void>> results;

        for (const auto& item : non_solid_files) {
            results.emplace_back(pool.enqueue([&, item] {
                verify_non_solid_file(archive_file, item, temp_dir, mismatches, checked_files, progress_counter, total_items_to_process, raw_output, use_basic_chars);
            }));
        }

        for (const auto& pair : solid_blocks) {
            results.emplace_back(pool.enqueue([&, pair] {
                verify_solid_block(archive_file, pair.second, temp_dir, mismatches, checked_files, progress_counter, total_items_to_process, raw_output, use_basic_chars);
            }));
        }

        for(auto && result : results)
            result.get();
    }
    
    if (total_items_to_process > 0 && !raw_output) {
        std::cout << std::endl;
    }

    fs::remove_all(temp_dir);

    if (checked_files == 0) {
        log("Verification complete. No files had hashes to check.", LOG_SUM);
    } else if (mismatches == 0) {
        log("Verification complete. All " + std::to_string(checked_files.load()) + " hashes matched.", LOG_SUCCESS);
    } else {
        log("Verification complete. Found " + std::to_string(mismatches.load()) + " hash mismatches out of " + std::to_string(checked_files.load()) + " checked files.", LOG_ERROR);
    }
}

} // namespace core
} // namespace prism

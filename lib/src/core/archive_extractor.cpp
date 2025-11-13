#include <prism/core/archive_extractor.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/compression.h>
#include <prism/hashing/openssl_hasher.h>
#include <fstream>
#include <iostream>
#include <set>
#include <sys/stat.h> // For mkdir

namespace prism {
namespace core {

// Helper from archive_writer, should be put in a shared utility file
void show_progress_bar(int current, int total, int width = 40);

void extract_archive(const std::string& archive_file, const std::string& output_dir, 
                     const std::vector<std::string>& files_to_extract, bool no_overwrite, bool no_verify) {
    std::vector<FileMetadata> items = read_archive_metadata(archive_file);
    
    std::vector<FileMetadata> items_to_extract;
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
                items_to_extract.push_back(item);
                continue;
            }
            for (const auto& dir : requested_dirs) {
                if (item.path.rfind(dir, 0) == 0) {
                    items_to_extract.push_back(item);
                    break; 
                }
            }
        }
        
        if (items_to_extract.empty()) {
            log("Warning: No matching files found in archive for the given paths", LOG_WARN);
            return;
        }
    } else {
        items_to_extract = items;
    }
    
    std::ifstream in(archive_file, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open archive: " + archive_file);
    }
    
    log("Extracting from archive: '" + archive_file + "'", LOG_INFO);
    
    int files_extracted = 0;
    int files_skipped = 0;
    uint64_t bytes_extracted = 0;
    int hash_mismatches = 0;
    int hashes_checked = 0;
    
    for (size_t i = 0; i < items_to_extract.size(); i++) {
        show_progress_bar(i + 1, items_to_extract.size());
        
        const auto& item = items_to_extract[i];
        std::string out_path = output_dir + "/" + item.path;

        if (no_overwrite && file_exists(out_path)) {
            log("Skipping existing file: '" + item.path + "'", LOG_VERBOSE);
            files_skipped++;
            continue;
        }
        
        size_t pos = out_path.find_last_of('/');
        if (pos != std::string::npos) {
            std::string dir = out_path.substr(0, pos);
            system(("mkdir -p \"" + dir + "\"" ).c_str());
        }
        
        in.seekg(item.data_start_offset);
        
        std::vector<char> compressed(item.compressed_size);
        in.read(compressed.data(), item.compressed_size);
        
        std::vector<char> decompressed = compression::decompress_data(compressed, 
                                                                      item.compression_type,
                                                                      item.file_size);
        
        std::ofstream out_file(out_path, std::ios::binary);
        if (!out_file) {
            log("Warning: Cannot create file: '" + out_path + "'", LOG_WARN);
            continue;
        }
        
        out_file.write(decompressed.data(), decompressed.size());
        out_file.close();
        
        files_extracted++;
        bytes_extracted += item.file_size;
        
        if (!no_verify && item.hash_type != HashType::NONE) {
            hashes_checked++;
            std::string calculated_hash = hashing::calculate_hash(out_path, item.hash_type);
            if (calculated_hash != item.file_hash) {
                hash_mismatches++;
                log("Hash mismatch for '" + item.path + "'. Data may be corrupted.", LOG_WARN);
            } else {
                log("Hash verified for '" + item.path + "'", LOG_VERBOSE);
            }
        }
        
        log("Extracted file '" + item.path + "' (" + format_size(item.file_size) + ")", LOG_INFO);
    }
    
    if (items_to_extract.size() > 0) std::cout << std::endl;
    
    log("Successfully extracted from archive '" + archive_file + "'", LOG_SUCCESS);
    log("Files extracted: " + std::to_string(files_extracted), LOG_SUM);
    if (files_skipped > 0) {
        log("Files skipped (already exist): " + std::to_string(files_skipped), LOG_SUM);
    }
    log("Total data extracted: " + format_size(bytes_extracted), LOG_SUM);
    
    if (!no_verify) {
        if (hashes_checked > 0) {
            if (hash_mismatches == 0) {
                log("Integrity Check: All hashes matched. File integrity verified.", LOG_SUM);
            } else {
                log("Integrity Check: " + std::to_string(hash_mismatches) + " hash mismatches found.", LOG_WARN);
            }
        } else {
            log("Integrity Check: No hashes were stored in the archive for verification.", LOG_SUM);
        }
    } else {
        log("Integrity Check: Skipped.", LOG_SUM);
    }
}

} // namespace core
} // namespace prism

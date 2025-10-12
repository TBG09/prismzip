#include <prism/core/archive_verifier.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/compression.h>
#include <prism/hashing/openssl_hasher.h>
#include "ui_utils.h"
#include <fstream>
#include <iostream>
#include <unistd.h> // for getpid()

namespace prism {
namespace core {void verify_archive(const std::string& archive_file) {
    if (!file_exists(archive_file)) {
        throw std::runtime_error("Archive file not found: " + archive_file);
    }
    
    log("Verifying archive integrity: '" + archive_file + "'", LOG_INFO);
    
    std::vector<FileMetadata> items = read_archive_metadata(archive_file);
    
    if (items.empty()) {
        log("Archive is empty.", LOG_WARN);
        return;
    }
    
    int verified_files = 0;
    int failed_files = 0;
    int total_checked = 0;
    bool has_hashing = false;
    
    for (const auto& item : items) {
        if (item.hash_type != HashType::NONE) {
            has_hashing = true;
            break;
        }
    }
    
    log("✓ Archive format validation passed", LOG_SUCCESS);
    log("✓ Successfully read metadata for " + std::to_string(items.size()) + " items", LOG_SUCCESS);
    
    if (!has_hashing) {
        log("! Hashing was not enabled in this archive - cannot verify data integrity", LOG_WARN);
        log("Archive verification completed with limitations:", LOG_SUM);
        log("✓ Archive format and structure are valid", LOG_SUM);
        log("! Data integrity cannot be verified without hashing", LOG_WARN);
        log("Consider recreating the archive with hash verification enabled (e.g., -H sha256)", LOG_SUM);
        return;
    }
    
    log("Hash verification enabled - verifying file integrity...", LOG_INFO);
    
    std::ifstream in(archive_file, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open archive for verification");
    }
    
    for (size_t i = 0; i < items.size(); i++) {
        show_progress_bar(i + 1, items.size());
        
        const auto& item = items[i];
        
        if (item.hash_type == HashType::NONE) {
            continue;
        }
        
        total_checked++;
        
        in.seekg(item.data_start_offset);
        std::vector<char> compressed(item.compressed_size);
        in.read(compressed.data(), item.compressed_size);
        
        if (in.gcount() != (long)item.compressed_size) {
            failed_files++;
            log("✗ Data truncation: '" + item.path + "'", LOG_WARN);
            continue;
        }
        
        std::vector<char> decompressed;
        try {
            decompressed = compression::decompress_data(compressed, item.compression_type, item.file_size);
        } catch (...) {
            failed_files++;
            log("✗ Decompression failed: '" + item.path + "'", LOG_WARN);
            continue;
        }
        
        if (decompressed.size() != item.file_size) {
            failed_files++;
            log("✗ Size mismatch: '" + item.path + "'", LOG_WARN);
            continue;
        }
        
        std::string calculated_hash = hashing::calculate_hash_from_data(decompressed, item.hash_type);
        
        if (calculated_hash == item.file_hash) {
            verified_files++;
            log("✓ Verified: '" + item.path + "'", LOG_VERBOSE);
        } else {
            failed_files++;
            log("✗ Hash mismatch: '" + item.path + "'", LOG_WARN);
            log("  Expected: " + item.file_hash, LOG_VERBOSE);
            log("  Got:      " + calculated_hash, LOG_VERBOSE);
        }
    }
    
    if (items.size() > 0) std::cout << std::endl << std::endl;
    
    if (failed_files == 0) {
        log("Archive verification completed successfully!", LOG_SUCCESS);
        log("✓ Archive format and structure are valid", LOG_SUM);
        log("✓ All " + std::to_string(verified_files) + " files passed hash verification", LOG_SUM);
    } else {
        log("Archive verification found issues!", LOG_WARN);
        if (verified_files > 0) {
            log("✓ Verified: " + std::to_string(verified_files) + " files", LOG_SUM);
        }
        log("✗ Failed verification: " + std::to_string(failed_files) + " files", LOG_WARN);
        log("Total hash-checked: " + std::to_string(total_checked) + " files", LOG_SUM);
    }
}

} // namespace core
} // namespace prism

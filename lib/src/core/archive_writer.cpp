#include <prism/core/archive_writer.h>
#include <prism/core/archive_reader.h> // For append
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/compression.h>
#include <prism/hashing/openssl_hasher.h>
#include <prism/core/ui_utils.h>
#include <fstream>
#include <iostream> // For progress bar
#include <iomanip>
#include <set>
#include <filesystem>
#include <sys/stat.h> // Required for stat struct

namespace fs = std::filesystem;

namespace prism {
namespace core {

std::vector<char> create_archive_header(const std::string& archive_path, CompressionType compression_type,
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

void create_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                   CompressionType comp_type, int level, HashType hash_type, 
                   bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes) {
    // Disk space verification
    uint64_t estimated_size = estimate_archive_size(archive_file, paths, comp_type, ignore_errors, exclude_patterns, use_full_path);
    uint64_t free_space = get_free_disk_space(fs::path(archive_file).parent_path().string());

    if (estimated_size > free_space) {
        std::string message = "Warning: Estimated archive size (" + format_size(estimated_size) + ") exceeds available disk space (" + format_size(free_space) + ") on target drive. Continue anyway?";
        if (!confirm_action(message, auto_yes)) {
            throw std::runtime_error("Archive creation cancelled by user.");
        }
    }

    std::ofstream out(archive_file, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot create archive file: " + archive_file);
    }
    
    out.write("PRZM", 4);
    uint16_t version = 1;
    out.write((char*)&version, 2);
    
    log("Created archive file named '" + archive_file + "'", LOG_INFO);
    
    int total_files = 0;
    uint64_t total_uncompressed = 0;
    uint64_t total_compressed = 0;
    
    for (const auto& path : paths) {
        if (!file_exists(path)) {
            if (ignore_errors) {
                log("Warning: Path not found: '" + path + "' (ignored)", LOG_WARN);
                continue;
            } else {
                throw std::runtime_error("Path not found: " + path);
            }
        }
        
        if (should_exclude(path, exclude_patterns)) {
            continue;
        }

        fs::path input_path_obj(path);
        fs::path base_path = input_path_obj.parent_path();
        if (base_path.empty()) {
            base_path = ".";
        }

        std::vector<std::string> files;
        if (is_directory(path)) {
            list_files_recursive(path, files, exclude_patterns);
        } else {
            files.push_back(path);
        }
        
        for (size_t i = 0; i < files.size(); i++) {
            show_progress_bar(i + 1, files.size());
            
            const std::string& file_path = files[i];
            
            std::string archive_path;
            if (use_full_path) {
                archive_path = get_absolute_path(file_path);
            } else {
                archive_path = fs::relative(file_path, base_path).string();
            }
            
            CompressionType actual_comp = should_compress(file_path, comp_type) ? comp_type : CompressionType::NONE;
            if (actual_comp != comp_type) {
                log("Skipping compression for already compressed file '" + file_path + "'", LOG_VERBOSE);
            }
            
            std::ifstream file(file_path, std::ios::binary);
            if (!file) {
                if (ignore_errors) {
                    log("Warning: Cannot open file: '" + file_path + "' (ignored)", LOG_WARN);
                    continue;
                } else {
                    throw std::runtime_error("Cannot open file: " + file_path);
                }
            }
            
            std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            
            std::string hash = hashing::calculate_hash(file_path, hash_type);
            std::vector<char> compressed = compression::compress_data(data, actual_comp, level);
            
            std::vector<char> header = create_archive_header(archive_path, actual_comp, level, 
                                                       hash_type, hash, data.size(), compressed.size());
            
            out.write(header.data(), header.size());
            out.write(compressed.data(), compressed.size());
            
            total_files++;
            total_uncompressed += data.size();
            total_compressed += compressed.size();
            
            log("  - Added file '" + archive_path + "' (" + format_size(data.size()) + ")", LOG_INFO);
        }
    }
    
    if (total_files > 0) std::cout << std::endl;
    
    log("Successfully created archive '" + archive_file + "'", LOG_SUCCESS);
    log("Items added: " + std::to_string(total_files) + " files", LOG_SUM);
    log("Total uncompressed data: " + format_size(total_uncompressed), LOG_SUM);
    log("Total compressed data: " + format_size(total_compressed), LOG_SUM);
    
    if (total_uncompressed > 0) {
        double ratio = 100.0 * (1.0 - (double)total_compressed / total_uncompressed);
        log("Compression ratio: " + std::to_string((int)ratio) + "%", LOG_SUM);
    }
}

void append_to_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                      CompressionType comp_type, int level, HashType hash_type, 
                      bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes) {
    if (!file_exists(archive_file)) {
        throw std::runtime_error("Archive file not found: " + archive_file);
    }
    
    // Disk space verification
    uint64_t estimated_size = estimate_archive_size(archive_file, paths, comp_type, ignore_errors, exclude_patterns, use_full_path);
    uint64_t free_space = get_free_disk_space(fs::path(archive_file).parent_path().string());

    if (estimated_size > free_space) {
        std::string message = "Warning: Estimated archive size (" + format_size(estimated_size) + ") exceeds available disk space (" + format_size(free_space) + ") on target drive. Continue anyway?";
        if (!confirm_action(message, auto_yes)) {
            throw std::runtime_error("Archive append cancelled by user.");
        }
    }

    std::vector<FileMetadata> existing_items = read_archive_metadata(archive_file);
    std::set<std::string> existing_paths;
    for (const auto& item : existing_items) {
        existing_paths.insert(item.path);
    }
    
    std::ofstream out(archive_file, std::ios::binary | std::ios::app);
    if (!out) {
        throw std::runtime_error("Cannot open archive file for appending: " + archive_file);
    }
    
    log("Appending to existing archive: '" + archive_file + "'", LOG_INFO);
    
    // ... (rest of the function is identical to create_archive, should be refactored)
    // For now, just copy-paste and adapt
    int total_files = 0;
    uint64_t total_uncompressed = 0;
    uint64_t total_compressed = 0;
    
    for (const auto& path : paths) {
        if (!file_exists(path)) {
            if (ignore_errors) {
                log("Warning: Path not found: '" + path + "' (ignored)", LOG_WARN);
                continue;
            } else {
                throw std::runtime_error("Path not found: " + path);
            }
        }
        
        if (should_exclude(path, exclude_patterns)) {
            continue;
        }

        fs::path input_path_obj(path);
        fs::path base_path = input_path_obj.parent_path();
        if (base_path.empty()) {
            base_path = ".";
        }

        std::vector<std::string> files;
        if (is_directory(path)) {
            list_files_recursive(path, files, exclude_patterns);
        } else {
            files.push_back(path);
        }
        
        for (size_t i = 0; i < files.size(); i++) {
            show_progress_bar(i + 1, files.size());
            
            const std::string& file_path = files[i];
            std::string archive_path;
            if (use_full_path) {
                archive_path = get_absolute_path(file_path);
            } else {
                archive_path = fs::relative(file_path, base_path).string();
            }
            
            if (existing_paths.count(archive_path)) {
                if (ignore_errors) {
                    log("Warning: File already exists in archive: '" + archive_path + "' (ignored)", LOG_WARN);
                    continue;
                } else {
                    throw std::runtime_error("File already exists in archive: " + archive_path);
                }
            }
            
            CompressionType actual_comp = should_compress(file_path, comp_type) ? comp_type : CompressionType::NONE;
            
            std::ifstream file(file_path, std::ios::binary);
            if (!file) {
                if (ignore_errors) {
                    log("Warning: Cannot open file: '" + file_path + "' (ignored)", LOG_WARN);
                    continue;
                } else {
                    throw std::runtime_error("Cannot open file: " + file_path);
                }
            }
            
            std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            
            std::string hash = hashing::calculate_hash(file_path, hash_type);
            std::vector<char> compressed = compression::compress_data(data, actual_comp, level);
            
            std::vector<char> header = create_archive_header(archive_path, actual_comp, level, 
                                                       hash_type, hash, data.size(), compressed.size());
            
            out.write(header.data(), header.size());
            out.write(compressed.data(), compressed.size());
            
            total_files++;
            total_uncompressed += data.size();
            total_compressed += compressed.size();
            
            log("  - Added file '" + archive_path + "' (" + format_size(data.size()) + ")", LOG_INFO);
        }
    }
    
    if (total_files > 0) std::cout << std::endl;
    
    log("Successfully appended to archive '" + archive_file + "'", LOG_SUCCESS);
    log("Items added: " + std::to_string(total_files) + " files", LOG_SUM);
    log("Total uncompressed data: " + format_size(total_uncompressed), LOG_SUM);
    log("Total compressed data: " + format_size(total_compressed), LOG_SUM);
}

uint64_t estimate_archive_size(const std::string& archive_file, const std::vector<std::string>& paths,
                             CompressionType comp_type,
                             bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path) {
    uint64_t total_uncompressed_size = 0;
    double compression_ratio = 1.0; // 1.0 means 100% of original size

    // Determine a rough compression ratio based on type
    switch (comp_type) {
        case CompressionType::NONE:
            compression_ratio = 1.0;
            break;
        case CompressionType::ZLIB:
        case CompressionType::BZIP2:
        case CompressionType::LZMA:
        case CompressionType::GZIP:
        case CompressionType::LZ4:
        case CompressionType::ZSTD:
        case CompressionType::BROTLI:
            compression_ratio = 0.5; // Assume 50% compression for now
            break;
        default:
            compression_ratio = 1.0; // Unknown type, assume no compression
            break;
    }

    for (const auto& path : paths) {
        if (!file_exists(path)) {
            if (ignore_errors) {
                log("Warning: Path not found for size estimation: '" + path + "' (ignored)", LOG_WARN);
                continue;
            } else {
                throw std::runtime_error("Path not found for size estimation: " + path);
            }
        }

        if (should_exclude(path, exclude_patterns)) {
            continue;
        }

        std::vector<std::string> files_to_process;
        if (is_directory(path)) {
            list_files_recursive(path, files_to_process, exclude_patterns);
        } else {
            files_to_process.push_back(path);
        }

        for (const auto& file_path : files_to_process) {
            struct stat st;
            if (stat(file_path.c_str(), &st) == 0) {
                total_uncompressed_size += st.st_size;
            } else {
                if (ignore_errors) {
                    log("Warning: Could not get size of file: '" + file_path + "' (ignored)", LOG_WARN);
                } else {
                    throw std::runtime_error("Could not get size of file: " + file_path);
                }
            }
        }
    }

    // Add some overhead for archive metadata (e.g., 1% of total size, or a fixed minimum)
    uint64_t metadata_overhead = std::max((uint64_t)1024, total_uncompressed_size / 100); // Min 1KB or 1%

    return (uint64_t)(total_uncompressed_size * compression_ratio) + metadata_overhead;
}

} // namespace core
} // namespace prism
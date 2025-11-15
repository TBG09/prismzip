#include <prism/core/archive_writer.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/compression.h>
#include <prism/hashing/openssl_hasher.h>
#include <prism/core/ui_utils.h>
#include <prism/core/thread_pool.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>
#include <filesystem>
#include <sys/stat.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <algorithm>
#include <cstring>

namespace fs = std::filesystem;

namespace prism {
namespace core {

namespace { // anonymous namespace
std::string get_archive_path(const std::string& file_path, const std::vector<std::string>& initial_paths, bool use_full_path) {
    if (use_full_path) {
        return get_absolute_path(file_path);
    }

    const std::string* matching_path_str = nullptr;
    for (const auto& p_str : initial_paths) {
        if (file_path.rfind(p_str, 0) == 0) {
            if (!matching_path_str || p_str.length() > matching_path_str->length()) {
                matching_path_str = &p_str;
            }
        }
    }

    fs::path base_path;
    if (matching_path_str) {
        base_path = fs::path(*matching_path_str).parent_path();
    } else {
        base_path = fs::path(file_path).parent_path();
    }

    if (base_path.empty()) {
        base_path = ".";
    }

    return fs::relative(file_path, base_path).string();
}
} // anonymous namespace

uint64_t estimate_archive_size(const std::string& archive_file, const std::vector<std::string>& paths,
                             CompressionType comp_type,
                             bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path);

inline std::vector<char> create_solid_file_metadata(const std::string& archive_path, HashType hash_type, const std::string& file_hash, uint64_t file_size,
                                                    uint64_t creation_time, uint64_t modification_time,
                                                    uint32_t permissions, uint32_t uid, uint32_t gid) {

    std::vector<char> metadata;

    uint32_t path_len = archive_path.size();
    metadata.resize(metadata.size() + 4);
    memcpy(&metadata[metadata.size() - 4], &path_len, 4);
    metadata.insert(metadata.end(), archive_path.begin(), archive_path.end());
    
    metadata.push_back(static_cast<uint8_t>(hash_type));
    
    uint16_t hash_len = file_hash.size();
    metadata.resize(metadata.size() + 2);
    memcpy(&metadata[metadata.size() - 2], &hash_len, 2);
    metadata.insert(metadata.end(), file_hash.begin(), file_hash.end());
    
    metadata.resize(metadata.size() + 8);
    memcpy(&metadata[metadata.size() - 8], &file_size, 8);

    metadata.resize(metadata.size() + 8);
    memcpy(&metadata[metadata.size() - 8], &creation_time, 8);
    metadata.resize(metadata.size() + 8);
    memcpy(&metadata[metadata.size() - 8], &modification_time, 8);
    metadata.resize(metadata.size() + 4);
    memcpy(&metadata[metadata.size() - 4], &permissions, 4);
    metadata.resize(metadata.size() + 4);
    memcpy(&metadata[metadata.size() - 4], &uid, 4);
    metadata.resize(metadata.size() + 4);
    memcpy(&metadata[metadata.size() - 4], &gid, 4);
    
    return metadata;
}



ArchiveCreationResult create_archive(const std::string& archive_file, const std::vector<std::string>& paths,

                   CompressionType comp_type, int level, HashType hash_type, 

                   bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes, int num_threads, bool raw_output, bool use_basic_chars, bool solid_mode) {



    uint64_t estimated_size = estimate_archive_size(archive_file, paths, comp_type, ignore_errors, exclude_patterns, use_full_path);

    fs::path p = archive_file;

    fs::path parent = p.parent_path();

    std::string path_for_space_check = parent.empty() ? "." : parent.string();

    uint64_t free_space = get_free_disk_space(path_for_space_check);



    if (estimated_size > free_space) {

        std::string message = "Warning: Estimated archive size (" + format_size(estimated_size) + ") exceeds available disk space (" + format_size(free_space) + ") on target drive. Continue anyway?";

        if (!confirm_action(message, auto_yes)) {

            throw std::runtime_error("Archive creation cancelled by user.");

        }

    }



    std::vector<std::string> all_files;

    for (const auto& path : paths) {

        if (!file_exists(path)) {

            if (ignore_errors) {

                log("Warning: Path not found: '" + path + "' (ignored)", LOG_WARN);

                continue;

            } else {

                throw std::runtime_error("Path not found: " + path);

            }

        }

        if (is_directory(path)) {

            list_files_recursive(path, all_files, exclude_patterns);

        } else {

            if (!should_exclude(path, exclude_patterns)) {

                all_files.push_back(path);

            }

        }

    }



    if (solid_mode) {

        log("Creating solid archive file named '" + archive_file + "'", LOG_INFO);



        std::vector<char> all_uncompressed_data;

                std::vector<char> metadata_block;

                uint64_t total_uncompressed_size = 0;

                int files_added = 0;

                auto start_time = std::chrono::steady_clock::now();

        

                for (const auto& file_path : all_files) {

                        std::string archive_path = get_archive_path(file_path, paths, use_full_path);



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



            all_uncompressed_data.insert(all_uncompressed_data.end(), data.begin(), data.end());

            total_uncompressed_size += data.size();



                        std::string hash = hashing::calculate_hash(file_path, hash_type);



            



                        FileMetadata file_props;



                        if (!get_file_properties(file_path, file_props)) {



                            if (ignore_errors) {



                                log("Warning: Failed to get properties for file: '" + file_path + "' (ignored)", LOG_WARN);



                                continue;



                            } else {



                                throw std::runtime_error("Failed to get properties for file: " + file_path);



                            }



                        }



            



                        std::vector<char> file_metadata = create_solid_file_metadata(archive_path, hash_type, hash, data.size(),



                                                                                     file_props.creation_time, file_props.modification_time,



                                                                                     file_props.permissions, file_props.uid, file_props.gid);

            metadata_block.insert(metadata_block.end(), file_metadata.begin(), file_metadata.end());

            

            files_added++;

            show_progress_bar(files_added, all_files.size(), archive_path, data.size(), 0, start_time, raw_output, use_basic_chars);

        }

        

        if (files_added > 0 && !raw_output) std::cout << std::endl;



        log("Compressing solid block...", LOG_INFO);

        std::vector<char> compressed_data = compression::compress_data(all_uncompressed_data, comp_type, level);

        log("Compression complete.", LOG_VERBOSE);



        std::ofstream out(archive_file, std::ios::binary);

        if (!out) {

            throw std::runtime_error("Cannot create archive file: " + archive_file);

        }



                out.write("PRZM", 4);



                uint16_t version = 1;



                out.write((char*)&version, 2);



                uint8_t flags = SOLID_ARCHIVE_FLAG;



                out.write((char*)&flags, 1);



                out.write((char*)&comp_type, 1);



                out.write((char*)&level, 1);



                



                uint64_t metadata_size = metadata_block.size();

        out.write((char*)&metadata_size, 8);

        out.write(metadata_block.data(), metadata_block.size());

        out.write(compressed_data.data(), compressed_data.size());

        

        log("Successfully created solid archive '" + archive_file + "'", LOG_SUCCESS);

        log("Items added: " + std::to_string(files_added) + " files", LOG_SUM);

        log("Total uncompressed data: " + format_size(total_uncompressed_size), LOG_SUM);

        log("Total compressed data: " + format_size(compressed_data.size()), LOG_SUM);

        if (total_uncompressed_size > 0) {

            double ratio = 100.0 * (1.0 - (double)compressed_data.size() / total_uncompressed_size);

            log("Compression ratio: " + std::to_string((int)ratio) + "%", LOG_SUM);

        }



        return { (long)files_added, total_uncompressed_size, compressed_data.size(), {} };



    } else {

        std::ofstream out(archive_file, std::ios::binary);

        if (!out) {

            throw std::runtime_error("Cannot create archive file: " + archive_file);

        }

        

        out.write("PRZM", 4);

        uint16_t version = 1;

        out.write((char*)&version, 2);

        uint8_t flags = 0;

        out.write((char*)&flags, 1);

        

                log("Created archive file named '" + archive_file + "' using " + std::to_string(num_threads) + " threads.", LOG_INFO);

        

                

        

                std::atomic<int> total_files = 0;

        

                std::atomic<uint64_t> total_uncompressed = 0;

        

                std::atomic<uint64_t> total_compressed = 0;

        

                std::atomic<int> progress_counter = 0;

        

                                auto start_time = std::chrono::steady_clock::now();

        

                

        

                        std::mutex out_mutex;

        

                        std::mutex cout_mutex;

        

                        std::vector<long long> durations_ms;

        

                

        

                        {

        

                            ThreadPool pool(num_threads);

        

                            std::vector<std::future<void>> results;

        

                

        

                            for (const auto& file_path : all_files) {

        

                                results.emplace_back(pool.enqueue([&, file_path] {

        

                                                        std::string archive_path = get_archive_path(file_path, paths, use_full_path);

        

                                    

        

                                    CompressionType actual_comp = should_compress(file_path, comp_type) ? comp_type : CompressionType::NONE;

        

                                    if (actual_comp != comp_type) {

        

                                        std::lock_guard<std::mutex> lock(cout_mutex);

        

                                        log("Skipping compression for already compressed file '" + file_path + "'", LOG_VERBOSE);

        

                                    }

        

                                    

        

                                    std::ifstream file(file_path, std::ios::binary);

        

                                    if (!file) {

        

                                        if (ignore_errors) {

        

                                            std::lock_guard<std::mutex> lock(cout_mutex);

        

                                            log("Warning: Cannot open file: '" + file_path + "' (ignored)", LOG_WARN);

        

                                            return;

        

                                        } else {

        

                                            throw std::runtime_error("Cannot open file: " + file_path);

        

                                        }

        

                                    }

        

                                    

        

                                    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        

                                    file.close();

        

                                    

        

                                    std::string hash = hashing::calculate_hash(file_path, hash_type);

        

                                    std::vector<char> compressed = compression::compress_data(data, actual_comp, level);

        

                                    

        

                                                        

        

                                    

        

                                                        FileMetadata file_props;

        

                                    

        

                                                        if (!get_file_properties(file_path, file_props)) {

        

                                    

        

                                                            if (ignore_errors) {

        

                                    

        

                                                                std::lock_guard<std::mutex> lock(cout_mutex);

        

                                    

        

                                                                log("Warning: Failed to get properties for file: '" + file_path + "' (ignored)", LOG_WARN);

        

                                    

        

                                                                return;

        

                                    

        

                                                            } else {

        

                                    

        

                                                                throw std::runtime_error("Failed to get properties for file: " + file_path);

        

                                    

        

                                                            }

        

                                    

        

                                                        }

        

                                    

        

                                    

        

                                    

        

                                                        std::vector<char> header = create_archive_header(archive_path, actual_comp, level, 

        

                                    

        

                                                                                                   hash_type, hash, data.size(), compressed.size(),

        

                                    

        

                                                                                                   file_props.creation_time, file_props.modification_time,

        

                                    

        

                                                                                                   file_props.permissions, file_props.uid, file_props.gid);

        

                                    

        

                                    {

        

                                        std::lock_guard<std::mutex> lock(out_mutex);

        

                                        out.write(header.data(), header.size());

        

                                        out.write(compressed.data(), compressed.size());

        

                                    }

        

                                    

        

                                    total_files++;

        

                                    total_uncompressed += data.size();

        

                                    total_compressed += compressed.size();

        

                                    

        

                                    {

        

                                        std::lock_guard<std::mutex> lock(cout_mutex);

        

                                        show_progress_bar(++progress_counter, all_files.size(), archive_path, data.size(), compressed.size(), start_time, raw_output, use_basic_chars);

        

                                    }

        

                                }));

        

                            }

        

                



            for(auto && result : results)

                result.get();

            

            durations_ms = pool.get_thread_durations();

        }

        

        if (total_files > 0 && !raw_output) std::cout << std::endl;

        

        log("Successfully created archive '" + archive_file + "'", LOG_SUCCESS);

        log("Items added: " + std::to_string(total_files.load()) + " files", LOG_SUM);

        log("Total uncompressed data: " + format_size(total_uncompressed.load()), LOG_SUM);

        log("Total compressed data: " + format_size(total_compressed.load()), LOG_SUM);

        

        if (total_uncompressed > 0) {

            double ratio = 100.0 * (1.0 - (double)total_compressed.load() / total_uncompressed.load());

            log("Compression ratio: " + std::to_string((int)ratio) + "%", LOG_SUM);

        }

        

        return {total_files.load(), total_uncompressed.load(), total_compressed.load(), durations_ms};

    }

}

ArchiveCreationResult append_to_archive(const std::string& archive_file, const std::vector<std::string>& paths,
                      CompressionType comp_type, int level, HashType hash_type, 
                      bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path, bool auto_yes, int num_threads, bool raw_output, bool use_basic_chars, bool solid_mode) {
    if (!file_exists(archive_file)) {
        throw std::runtime_error("Archive file not found: " + archive_file);
    }
    
    uint64_t estimated_size = estimate_archive_size(archive_file, paths, comp_type, ignore_errors, exclude_patterns, use_full_path);
    fs::path p = archive_file;
    fs::path parent = p.parent_path();
    std::string path_for_space_check = parent.empty() ? "." : parent.string();
    uint64_t free_space = get_free_disk_space(path_for_space_check);

    if (estimated_size > free_space) {
        std::string message = "Warning: Estimated archive size (" + format_size(estimated_size) + ") exceeds available disk space (" + format_size(free_space) + ") on target drive. Continue anyway?";
        if (!confirm_action(message, auto_yes)) {
            throw std::runtime_error("Archive append cancelled by user.");
        }
    }

    std::vector<std::string> all_files;
    for (const auto& path : paths) {
        if (!file_exists(path)) {
            if (ignore_errors) {
                log("Warning: Path not found: '" + path + "' (ignored)", LOG_WARN);
                continue;
            } else {
                throw std::runtime_error("Path not found: " + path);
            }
        }
        if (is_directory(path)) {
            list_files_recursive(path, all_files, exclude_patterns);
        } else {
            if (!should_exclude(path, exclude_patterns)) {
                all_files.push_back(path);
            }
        }
    }

    if (solid_mode) {
        if (is_solid_archive(archive_file)) {
            log("Warning: This will add another block to the end of the archive, this will make it no longer a solid block archive", LOG_WARN);
        }

        std::vector<FileMetadata> existing_items = read_archive_metadata(archive_file);
        std::set<std::string> existing_paths;
        for (const auto& item : existing_items) {
            existing_paths.insert(item.path);
        }

        log("Appending to archive '" + archive_file + "' in solid mode.", LOG_INFO);

        std::vector<char> all_uncompressed_data;
        std::vector<char> metadata_block;
        uint64_t total_uncompressed_size = 0;
        int files_added = 0;
        auto start_time = std::chrono::steady_clock::now();

        for (const auto& file_path : all_files) {
            std::string archive_path = get_archive_path(file_path, paths, use_full_path);

            if (existing_paths.count(archive_path)) {
                if (ignore_errors) {
                    log("Warning: File already exists in archive: '" + archive_path + "' (ignored)", LOG_WARN);
                    continue;
                } else {
                    throw std::runtime_error("File already exists in archive: " + archive_path);
                }
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

            all_uncompressed_data.insert(all_uncompressed_data.end(), data.begin(), data.end());
            total_uncompressed_size += data.size();

            std::string hash = hashing::calculate_hash(file_path, hash_type);
            
            FileMetadata file_props;
            if (!get_file_properties(file_path, file_props)) {
                if (ignore_errors) {
                    log("Warning: Failed to get properties for file: '" + file_path + "' (ignored)", LOG_WARN);
                    continue;
                } else {
                    throw std::runtime_error("Failed to get properties for file: " + file_path);
                }
            }

            std::vector<char> file_metadata = create_solid_file_metadata(archive_path, hash_type, hash, data.size(),
                                                                         file_props.creation_time, file_props.modification_time,
                                                                         file_props.permissions, file_props.uid, file_props.gid);
            metadata_block.insert(metadata_block.end(), file_metadata.begin(), file_metadata.end());
            files_added++;
            show_progress_bar(files_added, all_files.size(), archive_path, data.size(), 0, start_time, raw_output, use_basic_chars);
        }

        if (files_added > 0 && !raw_output) std::cout << std::endl;

        if (files_added == 0) {
            log("No new files to append.", LOG_INFO);
            return {0, 0, 0, {}};
        }

        log("Compressing solid block for appending...", LOG_INFO);
        std::vector<char> compressed_data = compression::compress_data(all_uncompressed_data, comp_type, level);
        log("Compression complete.", LOG_VERBOSE);

        std::ofstream out(archive_file, std::ios::binary | std::ios::app);
        if (!out) {
            throw std::runtime_error("Cannot open archive file for appending: " + archive_file);
        }

        out.write(SOLID_BLOCK_MAGIC, 4);
        out.write((char*)&comp_type, 1);
        out.write((char*)&level, 1);
        uint64_t metadata_size = metadata_block.size();
        out.write((char*)&metadata_size, 8);
        out.write(metadata_block.data(), metadata_block.size());
        out.write(compressed_data.data(), compressed_data.size());

        log("Successfully appended solid block to archive '" + archive_file + "'", LOG_SUCCESS);
        log("Items added: " + std::to_string(files_added) + " files", LOG_SUM);
        log("Total uncompressed data: " + format_size(total_uncompressed_size), LOG_SUM);
        log("Total compressed data: " + format_size(compressed_data.size()), LOG_SUM);

        return {(long)files_added, total_uncompressed_size, compressed_data.size(), {}};

    } else {
        std::vector<FileMetadata> existing_items = read_archive_metadata(archive_file);
        std::set<std::string> existing_paths;
        for (const auto& item : existing_items) {
            existing_paths.insert(item.path);
        }
        
        std::ofstream out(archive_file, std::ios::binary | std::ios::app);
        if (!out) {
            throw std::runtime_error("Cannot open archive file for appending: " + archive_file);
        }
        
        log("Appending to existing archive: '" + archive_file + "' using " + std::to_string(num_threads) + " threads.", LOG_INFO);
        
        std::atomic<int> total_files = 0;
        std::atomic<uint64_t> total_uncompressed = 0;
        std::atomic<uint64_t> total_compressed = 0;
        auto start_time = std::chrono::steady_clock::now();
        std::atomic<int> progress_counter = 0;

        std::mutex out_mutex;
        std::mutex cout_mutex;
        std::vector<long long> durations_ms;

        {
            ThreadPool pool(num_threads);
            std::vector<std::future<void>> results;

            for (const auto& file_path : all_files) {
                results.emplace_back(pool.enqueue([&, file_path] {
                    std::string archive_path = get_archive_path(file_path, paths, use_full_path);

                    if (existing_paths.count(archive_path)) {
                        if (ignore_errors) {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            log("Warning: File already exists in archive: '" + archive_path + "' (ignored)", LOG_WARN);
                            return;
                        } else {
                            throw std::runtime_error("File already exists in archive: " + archive_path);
                        }
                    }
                    
                    CompressionType actual_comp = should_compress(file_path, comp_type) ? comp_type : CompressionType::NONE;
                    
                    std::ifstream file(file_path, std::ios::binary);
                    if (!file) {
                        if (ignore_errors) {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            log("Warning: Cannot open file: '" + file_path + "' (ignored)", LOG_WARN);
                            return;
                        } else {
                            throw std::runtime_error("Cannot open file: " + file_path);
                        }
                    }
                    
                    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    file.close();
                    
                    std::string hash = hashing::calculate_hash(file_path, hash_type);
                    std::vector<char> compressed = compression::compress_data(data, actual_comp, level);
                    
                    
                    FileMetadata file_props;
                    if (!get_file_properties(file_path, file_props)) {
                        if (ignore_errors) {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            log("Warning: Failed to get properties for file: '" + file_path + "' (ignored)", LOG_WARN);
                            return;
                        } else {
                            throw std::runtime_error("Failed to get properties for file: " + file_path);
                        }
                    }

                    std::vector<char> header = create_archive_header(archive_path, actual_comp, level, 
                                                               hash_type, hash, data.size(), compressed.size(),
                                                               file_props.creation_time, file_props.modification_time,
                                                               file_props.permissions, file_props.uid, file_props.gid);
                    
                    {
                        std::lock_guard<std::mutex> lock(out_mutex);
                        out.write(header.data(), header.size());
                        out.write(compressed.data(), compressed.size());
                    }
                    
                    total_files++;
                    total_uncompressed += data.size();
                    total_compressed += compressed.size();
                    
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        show_progress_bar(++progress_counter, all_files.size(), archive_path, data.size(), compressed.size(), start_time, raw_output, use_basic_chars);
                    }
                }));
            }

            for(auto && result : results)
                result.get();
            
            durations_ms = pool.get_thread_durations();
        }
        
        if (total_files > 0 && !raw_output) std::cout << std::endl;
        
        log("Successfully appended to archive '" + archive_file + "'", LOG_SUCCESS);
        log("Items added: " + std::to_string(total_files.load()) + " files", LOG_SUM);
        log("Total uncompressed data: " + format_size(total_uncompressed.load()), LOG_SUM);
        log("Total compressed data: " + format_size(total_compressed.load()), LOG_SUM);

        return {total_files.load(), total_uncompressed.load(), total_compressed.load(), durations_ms};
    }
}

uint64_t estimate_archive_size(const std::string& archive_file, const std::vector<std::string>& paths,
                             CompressionType comp_type,
                             bool ignore_errors, const std::vector<std::string>& exclude_patterns, bool use_full_path) {
    uint64_t total_uncompressed_size = 0;
    double compression_ratio = 1.0;

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
            compression_ratio = 0.5;
            break;
        default:
            compression_ratio = 1.0;
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

    uint64_t metadata_overhead = std::max((uint64_t)1024, total_uncompressed_size / 100);

    return (uint64_t)(total_uncompressed_size * compression_ratio) + metadata_overhead;
}

} // namespace core
} // namespace prism

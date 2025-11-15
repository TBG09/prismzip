#include <cli.h>
#include <prism/core/types.h>
#include <prism/core/logging.h>
#include <prism/core/archive_lister.h>
#include <prism/core/archive_extractor.h>
#include <prism/core/archive_writer.h>
#include <prism/core/archive_remover.h>
#include <prism/core/archive_verifier.h>
#include <prism/core/result_types.h>
#include <prism/core/file_utils.h>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <any>

namespace prism {
namespace cli {

bool is_output_en = true;
bool is_sum_en = true;
bool is_verb_en = false;
bool is_warn_en = true;
bool is_err_en = true;
bool color_en = true;
bool is_extra_info_en = false;
bool is_raw_output_en = false;
bool use_basic_chars = false;

const std::string COLOR_RESET = "\033[0m";
const std::string COLOR_RED = "\033[91m";
const std::string COLOR_GREEN = "\033[92m";
const std::string COLOR_YELLOW = "\033[93m";
const std::string COLOR_BLUE = "\033[94m";
const std::string COLOR_CYAN = "\033[96m";

void output(const std::string& msg) {
    if (is_output_en) {
        if (color_en) std::cout << COLOR_CYAN << msg << COLOR_RESET << std::endl;
        else std::cout << msg << std::endl;
    }
}

void sumar(const std::string& msg) {
    if (is_sum_en) {
        if (color_en) std::cout << COLOR_YELLOW << msg << COLOR_RESET << std::endl;
        else std::cout << msg << std::endl;
    }
}

void warn(const std::string& msg) {
    if (is_warn_en) {
        if (color_en) std::cerr << COLOR_YELLOW << msg << COLOR_RESET << std::endl;
        else std::cerr << msg << std::endl;
    }
}

void err(const std::string& msg) {
    if (is_err_en) {
        if (color_en) std::cerr << COLOR_RED << msg << COLOR_RESET << std::endl;
        else std::cerr << msg << std::endl;
    }
}

void verb(const std::string& msg) {
    if (is_verb_en) {
        if (color_en) std::cout << COLOR_BLUE << msg << COLOR_RESET << std::endl;
        else std::cout << msg << std::endl;
    }
}

void success(const std::string& msg) {
    if (is_output_en) {
        if (color_en) std::cout << COLOR_GREEN << msg << COLOR_RESET << std::endl;
        else std::cout << msg << std::endl;
    }
}

void print_raw_summary(const std::string& msg);

void cli_log_handler(const std::string& msg, int level) {
    if (is_raw_output_en) {
        switch(static_cast<core::LogLevel>(level)) {
            case core::LOG_INFO:    
                break;
            case core::LOG_SUM:     print_raw_summary(msg); break;
            case core::LOG_WARN:    std::cerr << msg << std::endl; break;
            case core::LOG_ERROR:   std::cerr << msg << std::endl; break;
            case core::LOG_VERBOSE: 
                break;
            case core::LOG_DEBUG:   
                break; 
            case core::LOG_SUCCESS: 
                break;
        }
    } else {
        switch(static_cast<core::LogLevel>(level)) {
            case core::LOG_INFO:    output(msg); break;
            case core::LOG_SUM:     sumar(msg); break;
            case core::LOG_WARN:    warn(msg); break;
            case core::LOG_ERROR:   err(msg); break;
            case core::LOG_VERBOSE: verb(msg); break;
            case core::LOG_DEBUG:   if (is_verb_en) verb(msg); break; 
            case core::LOG_SUCCESS: success(msg); break;
        }
    }
}

void print_usage();
void print_command_help(const std::string& command);
void print_extra_info(const std::string& command, int num_threads, core::CompressionType comp_type, int comp_level, core::HashType hash_type, const std::any& result);


int run_cli(int argc, char* argv[]) {
    core::set_log_handler(cli_log_handler);

    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "-h" || command == "--help" || command == "help") {
        print_usage();
        return 0;
    }
    
    if (argc < 3 && command != "help") {
        print_command_help(command);
        return 1;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    std::string archive_file = (argc > 2) ? argv[2] : "";
    std::vector<std::string> paths;
    
    core::CompressionType comp_type = core::CompressionType::ZLIB;
    int comp_level = 9;
    core::HashType hash_type = core::HashType::NONE;
    std::string output_dir = ".";
    bool ignore_errors = false;
    bool use_full_path = false;
    std::vector<std::string> exclude_patterns;
    bool auto_yes = false;
        bool no_overwrite = false;
        bool no_verify = false;
        bool no_preserve_props = false; 
        int num_threads = 1;
        bool solid_mode = false;
        
        for (int i = 3; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-v") {
                is_verb_en = true;
            } else if (arg == "-i") {
                ignore_errors = true;
            } else if (arg == "-y") {
                auto_yes = true;
            } else if (arg == "-n") {
                no_verify = true;
            } else if (arg == "--no-overwrite") {
                no_overwrite = true;
            } else if (arg == "--no-preserve-props") { 
                no_preserve_props = true;
            } else if (arg == "--no-color") {
                color_en = false;
            } else if (arg == "--extra") {
                is_extra_info_en = true;
            } else if (arg == "--raw") {
                is_raw_output_en = true;
            } else if (arg == "--basic-chars") {
                use_basic_chars = true;
            } else if (arg == "-s" || arg == "--solid") {
                solid_mode = true;
            } else if (arg == "--full") {
                use_full_path = true;
            } else if (arg == "--exclude" && i + 1 < argc) {
                exclude_patterns.push_back(argv[++i]);
            } else if (arg == "--threads" && i + 1 < argc) {
                num_threads = std::atoi(argv[++i]);
                if (num_threads < 1) {
                    err("Error: Number of threads must be at least 1");
                    return 1;
                }
            } else if (arg == "-c" && i + 1 < argc) {
                std::string comp_str = argv[++i];
                if (core::COMPRESSION_MAP.count(comp_str)) {
                    comp_type = core::COMPRESSION_MAP.at(comp_str);
                } else {
                    err("Error: Invalid compression type '" + comp_str + "'");
                    return 1;
                }
            } else if (arg == "-l" && i + 1 < argc) {
                comp_level = std::atoi(argv[++i]);
                if (comp_level < 0 || comp_level > 9) {
                    err("Error: Compression level must be between 0 and 9");
                    return 1;
                }
            } else if (arg == "-H" && i + 1 < argc) {
                std::string hash_str = argv[++i];
                if (core::HASH_MAP.count(hash_str)) {
                    hash_type = core::HASH_MAP.at(hash_str);
                }
                else {
                    err("Error: Invalid hash type '" + hash_str + "'");
                    return 1;
                }
            } else if (arg == "-o" && i + 1 < argc) {
                output_dir = argv[++i];
            } else if (arg[0] != '-') {
                paths.push_back(arg);
            } else {
                err("Error: Unknown option '" + arg + "'");
                return 1;
            }
        }
        
        std::any result;
    
        try {
            if (command == "create") {
                if (paths.empty()) { print_command_help("create"); return 1; }
                result = core::create_archive(archive_file, paths, comp_type, comp_level, hash_type, ignore_errors, exclude_patterns, use_full_path, auto_yes, num_threads, is_raw_output_en, use_basic_chars, solid_mode);
            } else if (command == "append") {
                if (paths.empty()) { print_command_help("append"); return 1; }
                result = core::append_to_archive(archive_file, paths, comp_type, comp_level, hash_type, ignore_errors, exclude_patterns, use_full_path, auto_yes, num_threads, is_raw_output_en, use_basic_chars, solid_mode);
            } else if (command == "list") {
                core::list_archive(archive_file, false); 
                    } else if (command == "extract") {
                        result = core::extract_archive(archive_file, output_dir, paths, no_overwrite, no_verify, num_threads, is_raw_output_en, use_basic_chars, no_preserve_props);            } else if (command == "remove") {
                if (paths.empty()) { print_command_help("remove"); return 1; }
                core::remove_from_archive(archive_file, paths, ignore_errors, is_raw_output_en, use_basic_chars);
            } else if (command == "verify") {
                core::verify_archive(archive_file, is_raw_output_en, use_basic_chars);
            } else {
                err("Error: Unknown command '" + command + "'");
                print_usage();
                return 1;
            }
        } catch (const std::exception& e) {
            err(std::string("An error occurred: ") + e.what());
            return 1;
        }
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << std::endl;
    
        if (is_extra_info_en) {
            print_extra_info(command, num_threads, comp_type, comp_level, hash_type, result);
        }
    
        sumar("Total time elapsed: " + std::to_string(duration.count() / 1000) + "." + 
              std::to_string(duration.count() % 1000) + "s");
        
        return 0;
    }
    
    
    #include <any>
    #include <prism/core/file_utils.h>
    
    
    void print_extra_info(const std::string& command, int num_threads, core::CompressionType comp_type, int comp_level, core::HashType hash_type, const std::any& result) {
        if (is_raw_output_en) {
            std::cout << "threads_used=" << num_threads << std::endl;
            if (command == "create" || command == "append") {
                auto create_result = std::any_cast<core::ArchiveCreationResult>(result);
                std::cout << "compression_type=" << core::COMPRESSION_NAMES.at(comp_type) << std::endl;
                std::cout << "compression_level=" << comp_level << std::endl;
                std::cout << "hash_type=" << core::HASH_NAMES.at(hash_type) << std::endl;
                std::cout << "files_added=" << create_result.files_added << std::endl;
                std::cout << "uncompressed_size_bytes=" << create_result.total_uncompressed_size << std::endl;
                std::cout << "compressed_size_bytes=" << create_result.total_compressed_size << std::endl;
                if (create_result.total_uncompressed_size > 0) {
                    double ratio = 100.0 * (1.0 - (double)create_result.total_compressed_size / create_result.total_uncompressed_size);
                    std::cout << "compression_ratio_percent=" << (int)ratio << std::endl;
                }
                for (size_t i = 0; i < create_result.thread_durations_ms.size(); ++i) {
                    std::cout << "thread_" << i + 1 << "_duration_ms=" << create_result.thread_durations_ms[i] << std::endl;
                }
            } else if (command == "extract") {
                auto extract_result = std::any_cast<core::ArchiveExtractionResult>(result);
                std::cout << "files_extracted=" << extract_result.files_extracted << std::endl;
                std::cout << "files_skipped=" << extract_result.files_skipped << std::endl;
                std::cout << "bytes_extracted=" << extract_result.bytes_extracted << std::endl;
                std::cout << "hashes_checked=" << extract_result.hashes_checked << std::endl;
                std::cout << "hash_mismatches=" << extract_result.hash_mismatches << std::endl;
                for (size_t i = 0; i < extract_result.thread_durations_ms.size(); ++i) {
                    std::cout << "thread_" << i + 1 << "_duration_ms=" << extract_result.thread_durations_ms[i] << std::endl;
                }
            }
        } else {
            sumar("Extra Information:");
            sumar("  Operation Details:");
            sumar("    - Threads: " + std::to_string(num_threads));
    
            if (command == "create" || command == "append") {
                auto create_result = std::any_cast<core::ArchiveCreationResult>(result);
                sumar("    - Compression: " + core::COMPRESSION_NAMES.at(comp_type) + " (Level " + std::to_string(comp_level) + ")");
                sumar("    - Hashing: " + core::HASH_NAMES.at(hash_type));
                sumar("  Archive Statistics:");
                sumar("    - Files Added: " + std::to_string(create_result.files_added));
                sumar("    - Uncompressed Size: " + core::format_size(create_result.total_uncompressed_size));
                sumar("    - Compressed Size: " + core::format_size(create_result.total_compressed_size));
                if (create_result.total_uncompressed_size > 0) {
                    double ratio = 100.0 * (1.0 - (double)create_result.total_compressed_size / create_result.total_uncompressed_size);
                    sumar("    - Ratio: " + std::to_string((int)ratio) + "%");
                }
                sumar("  Thread Durations:");
                for (size_t i = 0; i < create_result.thread_durations_ms.size(); ++i) {
                    sumar("    - Thread " + std::to_string(i + 1) + ": " + std::to_string(create_result.thread_durations_ms[i]) + " ms");
                }
            } else if (command == "extract") {
                auto extract_result = std::any_cast<core::ArchiveExtractionResult>(result);
                sumar("  Extraction Statistics:");
                sumar("    - Files Extracted: " + std::to_string(extract_result.files_extracted));
                sumar("    - Files Skipped: " + std::to_string(extract_result.files_skipped));
                sumar("    - Total Data Extracted: " + core::format_size(extract_result.bytes_extracted));
                sumar("  Integrity Check:");
                sumar("    - Hashes Checked: " + std::to_string(extract_result.hashes_checked));
                sumar("    - Hash Mismatches: " + std::to_string(extract_result.hash_mismatches));
                sumar("  Thread Durations:");
                for (size_t i = 0; i < extract_result.thread_durations_ms.size(); ++i) {
                    sumar("    - Thread " + std::to_string(i + 1) + ": " + std::to_string(extract_result.thread_durations_ms[i]) + " ms");
                }
            }
        }
    }
    
    void print_raw_summary(const std::string& msg) {
        
    
        if (msg.rfind("Items added: ", 0) == 0) { 
            size_t start = msg.find(":") + 2;
            size_t end = msg.find(" files");
            std::cout << "items_added=" << msg.substr(start, end - start) << std::endl;
        } else if (msg.rfind("Total uncompressed data: ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "total_uncompressed_data=" << msg.substr(start) << std::endl;
        } else if (msg.rfind("Total compressed data: ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "total_compressed_data=" << msg.substr(start) << std::endl;
        } else if (msg.rfind("Compression ratio: ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "compression_ratio=" << msg.substr(start) << std::endl;
        } else if (msg.rfind("Files extracted: ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "files_extracted=" << msg.substr(start) << std::endl;
        } else if (msg.rfind("Files skipped (already exist): ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "files_skipped=" << msg.substr(start) << std::endl;
        } else if (msg.rfind("Total data extracted: ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "total_data_extracted=" << msg.substr(start) << std::endl;
        } else if (msg.rfind("Integrity Check: ", 0) == 0) {
            if (msg.find("All hashes matched") != std::string::npos) {
                std::cout << "integrity_check=verified" << std::endl;
            } else if (msg.find("hash mismatches found") != std::string::npos) {
                size_t start = msg.find(":") + 2;
                size_t end = msg.find(" hash mismatches");
                std::cout << "integrity_check_mismatches=" << msg.substr(start, end - start) << std::endl;
            } else if (msg.find("No hashes were stored") != std::string::npos) {
                std::cout << "integrity_check=no_hashes" << std::endl;
            } else if (msg.find("Skipped") != std::string::npos) {
                std::cout << "integrity_check=skipped" << std::endl;
            }
        } else if (msg.rfind("Total time elapsed: ", 0) == 0) {
            size_t start = msg.find(":") + 2;
            std::cout << "total_time_elapsed=" << msg.substr(start) << std::endl;
        }
        
    }
    
    void print_usage() {
        std::cout << "\n";
        if (color_en) {
            std::cout << COLOR_CYAN << "PrismZip Archive Utility - C++ Edition" << COLOR_RESET << "\n\n";
        } else {
            std::cout << "PrismZip Archive Utility - C++ Edition\n\n";
        }
        
        std::cout << "Usage: prismzip <command> <archive_file> [paths...] [options]\n\n";
        
        std::cout << "Commands:\n";
        std::cout << "  create     Create a new archive\n";
        std::cout << "  append     Append files to existing archive\n";
        std::cout << "  list       List archive contents\n";
        std::cout << "  extract    Extract files from archive\n";
        std::cout << "  remove     Remove files from archive\n";
        std::cout << "  verify     Verify archive integrity\n\n";
        
        std::cout << "Options:\n";
        std::cout << "  -c <type>      Compression: none, zlib, bzip2, lzma, gzip, lz4, zstd, brotli\n";
        std::cout << "  -l <level>     Compression level 0-9 (default: 9)\n";
        std::cout << "  -H <type>      Hash: none, md5, sha1, sha256, sha512, sha384, blake2b,\n";
        std::cout << "                 blake2s, sha3-256, sha3-512, ripemd160\n";
        std::cout << "  -s, --solid    Create a solid archive for better compression.\n";
        std::cout << "                 (This may make extraction slow, especially for individual files)\n";
        std::cout << "  -o <dir>       Output directory for extraction (default: .)\n";
        std::cout << "  -v             Verbose output\n";
        std::cout << "  -i             Ignore errors (skip files instead of stopping)\n";
        std::cout << "  -y             Auto-yes to all prompts (for automation)\n";
        std::cout << "  --threads <count> Number of threads to use (default: 1)\n";
        std::cout << "  --exclude <pattern>  Exclude files/folders matching pattern (supports * and ?)\n";
        std::cout << "  --full         Store full absolute paths in archive\n";
        std::cout << "  --no-color     Disable colored output\n";
        std::cout << "  --extra        Display extra information after operation\n";
        std::cout << "  --raw          Display raw, machine-readable output\n";
        std::cout << "  --basic-chars  Use basic characters for progress bar\n\n";
        
        
        std::cout << "Examples:\n";
        std::cout << "  # Create an archive with zlib compression and SHA256 hashing\n";
        std::cout << "  prismzip create backup.przm file.txt folder/ -c zlib -l 9 -H sha256\n\n";
        
        std::cout << "  # Create with full paths and exclusions\n";
        std::cout << "  prismzip create backup.przm /data/app/ --full --exclude \"*.tmp\" --exclude \"cache/*\"\n\n";
        std::cout << "  # List archive contents\n";
        std::cout << "  prismzip list backup.przm\n\n";
        
        std::cout << "  # Extract all files\n";
        std::cout << "  prismzip extract backup.przm -o output/\n\n";
        
        std::cout << "  # Extract specific files\n";
        std::cout << "  prismzip extract backup.przm file.txt folder/file2.txt -o restore/\n";
        std::cout << "  # Append new files to existing archive\n";
        std::cout << "  prismzip append backup.przm newfile.txt -c lzma -H sha256\n\n";
        
        std::cout << "  # Remove files from archive\n";
        std::cout << "  prismzip remove backup.przm file.txt folder/\n\n";
        
        std::cout << "  # Verify archive integrity (requires hashing)\n";
        std::cout << "  prismzip verify backup.przm\n\n";
        
        std::cout << "  # Verbose mode with best LZMA compression\n";
        std::cout << "  prismzip create archive.przm data/ -c lzma -l 9 -H sha512 -v\n\n";
    }
    
    void print_command_help(const std::string& command) {
        std::cout << "\n";
        if (color_en) {
            std::cout << COLOR_YELLOW << "Error: Incomplete command arguments" << COLOR_RESET << "\n\n";
        } else {
            std::cout << "Error: Incomplete command arguments\n\n";
        }
        
        if (command == "create") {
            std::cout << "Usage: prismzip create <archive_file> <paths...> [options]\n\n";
            std::cout << "Create a new archive from files and directories.\n\n";
            std::cout << "Required:\n";
            std::cout << "  <archive_file>  Name of the archive to create (e.g., backup.przm)\n";
            std::cout << "  <paths...>      One or more files or directories to add\n\n";
            std::cout << "Options:\n";
            std::cout << "  -c <type>       Compression type (default: zlib)\n";
            std::cout << "  -l <level>      Compression level 0-9 (default: 9)\n";
        std::cout << "  -s, --solid     Create a solid archive for better compression\n";
            std::cout << "  -H <type>       Hash algorithm for integrity checking\n";
            std::cout << "  -v              Verbose output\n";
            std::cout << "  -i              Ignore errors\n\n";
            std::cout << "Example:\n";
            std::cout << "  prismzip create backup.przm file.txt folder/ -c lzma -H sha256\n\n";
        } else if (command == "append") {
            std::cout << "Usage: prismzip append <archive_file> <paths...> [options]\n\n";
            std::cout << "Append files to an existing archive.\n\n";
            std::cout << "Required:\n";
            std::cout << "  <archive_file>  Existing archive file\n";
            std::cout << "  <paths...>      One or more files or directories to add\n\n";
            std::cout << "Options:\n";
            std::cout << "  -c <type>       Compression type (default: zlib)\n";
            std::cout << "  -l <level>      Compression level 0-9 (default: 9)\n";
            std::cout << "  -s, --solid     Append as a solid block\n";
            std::cout << "  -H <type>       Hash algorithm\n";
            std::cout << "  -v              Verbose output\n";
            std::cout << "  -i              Ignore errors (skip duplicates)\n\n";
            std::cout << "Example:\n";
            std::cout << "  prismzip append backup.przm newfile.txt -c zlib -H sha256\n\n";
        } else if (command == "extract") {
            std::cout << "Usage: prismzip extract <archive_file> [paths...] [options]\n\n";
            std::cout << "Extract files from an archive.\n\n";
            std::cout << "Required:\n";
            std::cout << "  <archive_file>  Archive file to extract from\n\n";
            std::cout << "Optional:\n";
            std::cout << "  [paths...]      Specific files to extract (default: all)\n";
            std::cout << "  -o <dir>        Output directory (default: current directory)\n";
            std::cout << "  -v              Verbose output\n";
            std::cout << "  -n              No verification on extraction\n";
            std::cout << "  --no-overwrite  Do not overwrite existing files on extraction\n\n";
            std::cout << "Examples:\n";
            std::cout << "  prismzip extract backup.przm -o output/\n";
            std::cout << "  prismzip extract backup.przm file.txt folder/file2.txt -o restore/\n";
            std::cout << "  prismzip extract backup.przm -o output/ --no-overwrite -n\n\n";
        } else if (command == "remove") {
            std::cout << "Usage: prismzip remove <archive_file> <paths...> [options]\n\n";
            std::cout << "Remove files from an archive.\n\n";
            std::cout << "Required:\n";
            std::cout << "  <archive_file>  Archive file to modify\n";
            std::cout << "  <paths...>      One or more file paths to remove\n\n";
            std::cout << "Options:\n";
            std::cout << "  -v              Verbose output\n";
            std::cout << "  -i              Ignore errors\n\n";
            std::cout << "Example:\n";
            std::cout << "  prismzip remove backup.przm oldfile.txt temp/\n\n";
        } else if (command == "list") {
            std::cout << "Usage: prismzip list <archive_file> [options]\n\n";
            std::cout << "List contents of an archive.\n\n";
            std::cout << "Required:\n";
            std::cout << "  <archive_file>  Archive file to list\n\n";
            std::cout << "Options:\n";
            std::cout << "  -v              Verbose output\n\n";
            std::cout << "Example:\n";
            std::cout << "  prismzip list backup.przm\n\n";
        } else if (command == "verify") {
            std::cout << "Usage: prismzip verify <archive_file> [options]\n\n";
            std::cout << "Verify integrity of an archive.\n\n";
            std::cout << "Required:\n";
            std::cout << "  <archive_file>  Archive file to verify\n\n";
            std::cout << "Options:\n";
            std::cout << "  -v              Verbose output\n\n";
            std::cout << "Note: Archive must have been created with hash verification enabled.\n\n";
            std::cout << "Example:\n";
            std::cout << "  prismzip verify backup.przm -v\n\n";
        }
    }

} // namespace cli
} // namespace prism

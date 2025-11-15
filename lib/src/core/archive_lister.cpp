#include <prism/core/archive_lister.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <iostream>

namespace prism {
namespace core {

void list_archive(const std::string& archive_file, bool raw_list_mode) {
    std::vector<FileMetadata> items = read_archive_metadata(archive_file);
    
    if (!raw_list_mode) {
        log("Listing contents of '" + archive_file + "'..", LOG_SUM);
    }
    
    if (items.empty()) {
        if (!raw_list_mode) log("Archive is empty.", LOG_INFO);
        return;
    }
    
    uint64_t total_uncompressed = 0;
    uint64_t total_compressed = 0;
    
    for (const auto& item : items) {
        std::string comp_name = COMPRESSION_NAMES.at(item.compression_type);
        
        if (raw_list_mode) {
            std::cout << item.path << "\t" << item.file_size << "\t" << comp_name << "\n";
        } else {
            double ratio = 0;
            if (item.file_size > 0) {
                ratio = 100.0 * (1.0 - (double)item.compressed_size / item.file_size);
            }
            
            std::string hash_info = "";
            if (item.hash_type != HashType::NONE) {
                std::string hash_name = HASH_NAMES.at(item.hash_type);
                hash_info = " [" + hash_name + "]";
            }
            
            log(item.path + " - " + format_size(item.file_size) + 
                  " (" + comp_name + ", " + std::to_string((int)ratio) + "% saved" + hash_info + ")", LOG_INFO);
        }
        
        total_uncompressed += item.file_size;
        total_compressed += item.compressed_size;
    }
    
    if (!raw_list_mode) {
        std::cout << std::endl;
        log("Archive Summary:", LOG_SUM);
        log("  Files: " + std::to_string(items.size()), LOG_SUM);
        log("  Total size: " + format_size(total_uncompressed), LOG_SUM);
        log("  Compressed size: " + format_size(total_compressed), LOG_SUM);
        
        if (total_uncompressed > 0) {
            double ratio = 100.0 * (1.0 - (double)total_compressed / total_uncompressed);
            log("  Overall compression: " + std::to_string((int)ratio) + "%", LOG_SUM);
        }
    }
}

} // namespace core
} // namespace prism

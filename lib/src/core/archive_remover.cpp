#include <prism/core/archive_remover.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include "ui_utils.h"
#include <set>
#include <fstream>
#include <iostream> // For progress bar

namespace prism {
namespace core {

void remove_from_archive(const std::string& archive_file, const std::vector<std::string>& paths_to_remove, bool ignore_errors) {
    if (!file_exists(archive_file)) {
        throw std::runtime_error("Archive file not found: " + archive_file);
    }
    
    std::vector<FileMetadata> all_items = read_archive_metadata(archive_file);
    std::vector<FileMetadata> items_to_keep;
    
    std::set<std::string> remove_set(paths_to_remove.begin(), paths_to_remove.end());
    
    int total_removed = 0;
    uint64_t bytes_removed = 0;
    
    log("Removing from archive: '" + archive_file + "'", LOG_INFO);
    
    for (const auto& item : all_items) {
        bool should_remove = false;
        
        if (remove_set.count(item.path)) {
            should_remove = true;
        } else {
            for (const auto& remove_path : paths_to_remove) {
                if (item.path.rfind(remove_path, 0) == 0) {
                    should_remove = true;
                    break;
                }
            }
        }
        
        if (should_remove) {
            total_removed++;
            bytes_removed += item.file_size;
            log("  - Removing: '" + item.path + "'", LOG_INFO);
        } else {
            items_to_keep.push_back(item);
        }
    }
    
    if (total_removed == 0) {
        if (ignore_errors) {
            log("Warning: No matching paths found to remove", LOG_WARN);
            return;
        } else {
            throw std::runtime_error("No matching paths found to remove");
        }
    }
    
    std::string temp_file = archive_file + ".tmp";
    std::ofstream out(temp_file, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot create temporary file");
    }
    
    std::ifstream in(archive_file, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot read archive file");
    }
    
    out.write("PRZM", 4);
    uint16_t version = 1;
    out.write((char*)&version, 2);
    
    for (size_t i = 0; i < items_to_keep.size(); i++) {
        show_progress_bar(i + 1, items_to_keep.size());
        
        const auto& item = items_to_keep[i];
        
        in.seekg(item.header_start_offset);
        
        size_t header_size = item.data_start_offset - item.header_start_offset;
        size_t total_size = header_size + item.compressed_size;
        
        std::vector<char> data(total_size);
        in.read(data.data(), total_size);
        out.write(data.data(), total_size);
    }
    
    if (items_to_keep.size() > 0) std::cout << std::endl;
    
    in.close();
    out.close();
    
    if (remove(archive_file.c_str()) != 0) {
        throw std::runtime_error("Cannot remove original archive file");
    }
    
    if (rename(temp_file.c_str(), archive_file.c_str()) != 0) {
        throw std::runtime_error("Cannot rename temporary file");
    }
    
    log("Successfully removed items from archive '" + archive_file + "'", LOG_SUCCESS);
    log("Items removed: " + std::to_string(total_removed), LOG_SUM);
    log("Bytes freed: " + format_size(bytes_removed), LOG_SUM);
}

} // namespace core
} // namespace prism

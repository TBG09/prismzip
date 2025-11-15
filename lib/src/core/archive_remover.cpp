#include <prism/core/archive_remover.h>
#include <prism/core/archive_reader.h>
#include <prism/core/archive_writer.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/core/ui_utils.h>
#include <fstream>
#include <iostream>
#include <set>
#include <cstdio>
#include <stdexcept>

namespace prism {
namespace core {

void remove_from_archive(const std::string& archive_file, const std::vector<std::string>& files_to_remove, bool ignore_errors, bool raw_output, bool use_basic_chars) {
    if (!file_exists(archive_file)) {
        throw std::runtime_error("Archive file not found: " + archive_file);
    }

    std::vector<FileMetadata> all_items = read_archive_metadata(archive_file);
    std::vector<FileMetadata> items_to_keep;
    std::set<std::string> remove_set(files_to_remove.begin(), files_to_remove.end());

    for (const auto& item : all_items) {
        if (remove_set.find(item.path) == remove_set.end()) {
            items_to_keep.push_back(item);
        } else {
            log("Marked for removal: '" + item.path + "'", LOG_INFO);
        }
    }

    if (items_to_keep.size() == all_items.size()) {
        log("No matching files found in archive to remove.", LOG_WARN);
        if (!ignore_errors && !files_to_remove.empty()) {
             throw std::runtime_error("No files matched for removal.");
        }
        return;
    }

    std::string temp_archive_file = archive_file + ".tmp";
    std::ofstream temp_out(temp_archive_file, std::ios::binary);
    if (!temp_out) {
        throw std::runtime_error("Could not create temporary archive file.");
    }

    temp_out.write("PRZM", 4);
    uint16_t version = 1;
    temp_out.write((char*)&version, 2);

    log("Rebuilding archive...", LOG_INFO);

    std::ifstream original_in(archive_file, std::ios::binary);
    if (!original_in) {
        throw std::runtime_error("Could not open original archive for reading.");
    }

    for (size_t i = 0; i < items_to_keep.size(); ++i) {
        const auto& item = items_to_keep[i];
        
        std::vector<char> header = create_archive_header(item.path, item.compression_type, item.level,
                                                       item.hash_type, item.file_hash, item.file_size,
                                                       item.compressed_size, item.creation_time,
                                                       item.modification_time, item.permissions,
                                                       item.uid, item.gid);
        temp_out.write(header.data(), header.size());

        original_in.seekg(item.data_start_offset);
        std::vector<char> buffer(item.compressed_size);
        original_in.read(buffer.data(), item.compressed_size);
        temp_out.write(buffer.data(), buffer.size());
        
        show_progress_bar(i + 1, items_to_keep.size(), item.path, item.file_size, raw_output, use_basic_chars);
    }
    if (!items_to_keep.empty() && !raw_output) {
        std::cout << std::endl;
    }

    original_in.close();
    temp_out.close();

    if (std::rename(temp_archive_file.c_str(), archive_file.c_str()) != 0) {
        throw std::runtime_error("Failed to replace original archive with rebuilt one.");
    }

    log("Successfully removed " + std::to_string(all_items.size() - items_to_keep.size()) + " file(s).", LOG_SUCCESS);
}

} // namespace core
} // namespace prism

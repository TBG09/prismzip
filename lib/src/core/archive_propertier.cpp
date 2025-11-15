#include <prism/core/archive_propertier.h>
#include <prism/core/archive_reader.h>
#include <prism/core/file_utils.h>
#include <prism/core/logging.h>
#include <prism/core/ui_utils.h>
#include <iostream>
#include <iomanip>
#include <ctime>

namespace prism {
namespace core {

void print_properties(const FileMetadata& item) {
    log("Properties for: " + item.path, LOG_SUM);
    log("  Size: " + format_size(item.file_size), LOG_SUM);
    log("  Compressed Size: " + format_size(item.compressed_size), LOG_SUM);
    log("  Compression: " + COMPRESSION_NAMES.at(item.compression_type), LOG_SUM);
    log("  Compression Level: " + std::to_string(item.level), LOG_SUM);
    log("  Hash: " + HASH_NAMES.at(item.hash_type), LOG_SUM);
    log("  File Hash: " + item.file_hash, LOG_SUM);

    std::time_t creation_time = item.creation_time;
    std::time_t mod_time = item.modification_time;
    log("  Creation Time: " + std::string(std::asctime(std::localtime(&creation_time))), LOG_SUM);
    log("  Modification Time: " + std::string(std::asctime(std::localtime(&mod_time))), LOG_SUM);

    log("  Permissions: " + std::to_string(item.permissions), LOG_SUM);
    log("  UID: " + std::to_string(item.uid), LOG_SUM);
    log("  GID: " + std::to_string(item.gid), LOG_SUM);
}

void get_properties(const std::string& archive_file, const std::string& path_in_archive, bool auto_yes) {
    if (is_solid_archive(archive_file)) {
        log("Warning: This is a solid archive. To get properties of a specific file, the entire archive may need to be decompressed.", LOG_WARN);
        if (!confirm_action("Do you want to proceed?", auto_yes)) {
            log("Operation cancelled.", LOG_INFO);
            return;
        }
    }

    std::vector<FileMetadata> items = read_archive_metadata(archive_file);
    bool found = false;

    for (const auto& item : items) {
        if (item.path == path_in_archive) {
            print_properties(item);
            found = true;
            break;
        }
    }

    if (!found) {
        log("Error: File not found in archive: " + path_in_archive, LOG_ERROR);
    }
}

} // namespace core
} // namespace prism

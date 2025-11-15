#include <prism/core/file_utils.h>
#include <prism/core/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <regex>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <fcntl.h>

#include <prism/core/logging.h>

namespace fs = std::filesystem;

namespace prism {
namespace core {

std::string format_size(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unit < 5) {
        size /= 1024.0;
        unit++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return ss.str();
}

bool file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool is_directory(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

std::string get_extension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        std::string ext = path.substr(pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    return "";
}

bool should_compress(const std::string& file_path, CompressionType compression_type) {
    if (compression_type == CompressionType::NONE) return false;
    std::string ext = get_extension(file_path);
    return COMPRESSED_EXTENSIONS.find(ext) == COMPRESSED_EXTENSIONS.end();
}

bool match_pattern(const std::string& path, const std::string& pattern) {
    log("match_pattern: path='" + path + "', original_pattern='" + pattern + "'", LOG_VERBOSE);
    std::string regex_pattern_str;
    for (char c : pattern) {
        if (c == '.' || c == '^' || c == ' || c == '|' || c == '(' || c == ')' ||
            c == '[' || c == ']' || c == '{' || c == '}' || c == '*' || c == '+' ||
            c == '?' || c == '\\') {
            regex_pattern_str += '\\';
        }
        regex_pattern_str += c;
    }

    regex_pattern_str = std::regex_replace(regex_pattern_str, std::regex("\\*"), ".*");
    regex_pattern_str = std::regex_replace(regex_pattern_str, std::regex("\\?"), ".");

    log("match_pattern: generated regex_pattern='" + regex_pattern_str + "'", LOG_VERBOSE);
    
    try {
        log("match_pattern: compiling regex: '" + regex_pattern_str + "'", LOG_VERBOSE);
        std::regex re(regex_pattern_str);
        bool result = std::regex_search(path, re);
        log("match_pattern: regex_search result=" + std::to_string(result), LOG_VERBOSE);
        return result;
    } catch (const std::regex_error& e) {
        log("match_pattern: regex_error: " + std::string(e.what()), LOG_WARN);
        bool result = path.find(pattern) != std::string::npos;
        log("match_pattern: fallback substring search result=" + std::to_string(result), LOG_VERBOSE);
        return result;
    } catch (...) {
        log("match_pattern: unknown error in regex processing", LOG_WARN);
        bool result = path.find(pattern) != std::string::npos;
        log("match_pattern: fallback substring search result=" + std::to_string(result), LOG_VERBOSE);
        return result;
    }
}

bool should_exclude(const std::string& path, const std::vector<std::string>& exclude_patterns) {
    log("should_exclude: path='" + path + "', exclude_patterns_count=" + std::to_string(exclude_patterns.size()), LOG_VERBOSE);
    for (const auto& pattern : exclude_patterns) {
        if (match_pattern(path, pattern)) {
            log("Excluding '" + path + "' (matches pattern: " + pattern + ")", LOG_VERBOSE);
            return true;
        }
        
        if (path.rfind(pattern, 0) == 0) {
            log("Excluding '" + path + "' (under excluded path: " + pattern + ")", LOG_VERBOSE);
            return true;
        }
    }
    return false;
}

std::string get_absolute_path(const std::string& path) {
    char* real = realpath(path.c_str(), nullptr);
    if (real) {
        std::string result(real);
        free(real);
        return result;
    }
    return path;
}

void list_files_recursive(const std::string& dir_path, std::vector<std::string>& files, const std::vector<std::string>& exclude_patterns) {
    DIR* dir = opendir(dir_path.c_str());
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        std::string full_path = dir_path + "/" + name;
        
        if (should_exclude(full_path, exclude_patterns)) {
            continue;
        }
        
        if (is_directory(full_path)) {
            list_files_recursive(full_path, files, exclude_patterns);
        } else {
            files.push_back(full_path);
        }
    }
    closedir(dir);
}

uint64_t get_free_disk_space(const std::string& path) {
    try {
        std::error_code ec;
        const fs::space_info si = fs::space(path, ec);
        if (ec) {
            log("Warning: Could not get free disk space for path '" + path + "': " + ec.message(), LOG_WARN);
            return 0;
        }
        return si.available;
    } catch (const fs::filesystem_error& e) {
        log("Warning: Could not get free disk space for path '" + path + "': " + e.what(), LOG_WARN);
        return 0;
    }
}

bool get_file_properties(const std::string& path, FileMetadata& metadata) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        log("Failed to get properties for file: " + path, LOG_ERROR);
        return false;
    }

    metadata.modification_time = st.st_mtime;

    #ifdef __APPLE__
        metadata.creation_time = st.st_birthtime;
    #else
        metadata.creation_time = st.st_ctime;
    #endif

    metadata.permissions = st.st_mode;
    metadata.uid = st.st_uid;
    metadata.gid = st.st_gid;

    return true;
}

bool set_file_properties(const std::string& path, const FileMetadata& metadata) {
    if (chmod(path.c_str(), metadata.permissions) != 0) {
        log("Failed to set permissions for file: " + path, LOG_WARN);
    }

    if (getuid() == 0 || (metadata.uid != 0 && metadata.gid != 0)) {
        if (chown(path.c_str(), metadata.uid, metadata.gid) != 0) {
            log("Failed to set ownership for file: " + path, LOG_WARN);
        }
    }

    struct timespec times[2];
    times[0].tv_sec = metadata.modification_time;
    times[0].tv_nsec = 0;
    times[1].tv_sec = metadata.modification_time;
    times[1].tv_nsec = 0;

    if (utimensat(AT_FDCWD, path.c_str(), times, 0) != 0) {
        log("Failed to set modification time for file: " + path, LOG_WARN);
    }
    
    return true;
}

} // namespace core
} // namespace prism
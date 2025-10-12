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
#include <cstdlib> // For realpath

#include <prism/core/logging.h>

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
    std::string regex_pattern = pattern;
    regex_pattern = std::regex_replace(regex_pattern, std::regex(R"([.^$|()[\\\]{}*+?])"), R"(\\\\\$&)");
    regex_pattern = std::regex_replace(regex_pattern, std::regex(R"(\\\\*)"), ".*");
    regex_pattern = std::regex_replace(regex_pattern, std::regex(R"(\\\\?)"), ".");
    
    try {
        std::regex re(regex_pattern);
        return std::regex_search(path, re);
    } catch (...) {
        return path.find(pattern) != std::string::npos;
    }
}

bool should_exclude(const std::string& path, const std::vector<std::string>& exclude_patterns) {
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

} // namespace core
} // namespace prism
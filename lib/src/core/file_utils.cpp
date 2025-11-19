#include <prism/core/file_utils.h>
#include <prism/core/types.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <regex>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <system_error>

#include <prism/core/logging.h>

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#include <sddl.h>
#include <AclAPI.h>
#pragma comment(lib, "advapi32.lib")
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#endif

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
    return fs::exists(path);
}

bool is_directory(const std::string& path) {
    return fs::is_directory(path);
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
        if (c == '.' || c == '^' || c == '$' || c == '|' || c == '(' || c == ')' ||
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
    return fs::absolute(path).string();
}

void list_files_recursive(const std::string& dir_path, std::vector<std::string>& files, const std::vector<std::string>& exclude_patterns) {
    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        std::string full_path = entry.path().string();
        
        if (should_exclude(full_path, exclude_patterns)) {
            continue;
        }
        
        if (fs::is_regular_file(entry.status())) {
            files.push_back(full_path);
        }
    }
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
    std::error_code ec;
    fs::file_status status = fs::status(path, ec);
    if (ec) {
        log("Failed to get properties for file: " + path + " - " + ec.message(), LOG_ERROR);
        return false;
    }

    auto ftime = fs::last_write_time(path, ec);
    if (!ec) {
        metadata.modification_time = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
    } else {
        log("Warning: Failed to get modification time for file: " + path + " - " + ec.message(), LOG_WARN);
    }

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        FILETIME ftCreate, ftAccess, ftWrite;
        if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
            ULARGE_INTEGER uli;
            uli.LowPart = ftCreate.dwLowDateTime;
            uli.HighPart = ftCreate.dwHighDateTime;
            metadata.creation_time = uli.QuadPart / 10000000ULL - 11644473600ULL;
        } else {
            log("Warning: Failed to get creation time for file (WinAPI): " + path, LOG_WARN);
        }
        CloseHandle(hFile);
    } else {
        log("Warning: Failed to open file for creation time (WinAPI): " + path, LOG_WARN);
    }

    PSECURITY_DESCRIPTOR pSD = nullptr;
    PSID pOwnerSid = nullptr;
    if (GetNamedSecurityInfoA(path.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pOwnerSid, NULL, NULL, NULL, &pSD) == ERROR_SUCCESS) {
        PSID adminSid = nullptr;
        SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&sidAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminSid)) {
            if (EqualSid(pOwnerSid, adminSid)) {
                metadata.uid = 0;
                metadata.gid = 0;
            } else {
                metadata.uid = 1000;
                metadata.gid = 1000;
            }
            FreeSid(adminSid);
        }
        if (pSD) LocalFree(pSD);
    }
    metadata.permissions = static_cast<uint32_t>(fs::status(path).permissions());
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        log("Failed to get properties for file: " + path, LOG_ERROR);
        return false;
    }

    #ifdef __APPLE__
        metadata.creation_time = st.st_birthtime;
    #else
        metadata.creation_time = st.st_ctime;
    #endif

    metadata.permissions = st.st_mode;
    metadata.uid = st.st_uid;
    metadata.gid = st.st_gid;
#endif

    return true;
}

bool set_file_properties(const std::string& path, const FileMetadata& metadata) {
    std::error_code ec;

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        ULARGE_INTEGER uli;
        uli.QuadPart = (uint64_t)metadata.modification_time * 10000000ULL + 11644473600ULL;
        FILETIME ftWrite;
        ftWrite.dwLowDateTime = uli.LowPart;
        ftWrite.dwHighDateTime = uli.HighPart;
        if (!SetFileTime(hFile, NULL, NULL, &ftWrite)) {
            log("Failed to set modification time for file (WinAPI): " + path, LOG_WARN);
        }
        CloseHandle(hFile);
    } else {
        log("Failed to open file for setting modification time (WinAPI): " + path, LOG_WARN);
    }

    if (metadata.uid == 0) {
        PSID adminSid = nullptr;
        SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&sidAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminSid)) {
            DWORD result = SetNamedSecurityInfoA((LPSTR)path.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, adminSid, NULL, NULL, NULL);
            if (result != ERROR_SUCCESS) {
                log("Warning: Failed to set owner to Administrators group for file: " + path + ". Error code: " + std::to_string(result), LOG_WARN);
            }
            FreeSid(adminSid);
        }
    }
    fs::permissions(path, static_cast<fs::perms>(metadata.permissions), ec);
#else
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
#endif
    
    return true;
}

} // namespace core
} // namespace prism
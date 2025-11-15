#ifndef PRISM_CORE_LOGGING_H
#define PRISM_CORE_LOGGING_H

#include <string>
#include <functional>

namespace prism {
namespace core {

using LogHandler = std::function<void(const std::string&, int)>;

enum LogLevel {
    LOG_INFO,
    LOG_SUM, 
    LOG_WARN,
    LOG_ERROR,
    LOG_VERBOSE,
    LOG_DEBUG, 
    LOG_SUCCESS
};

void set_log_handler(LogHandler handler);

void log(const std::string& msg, LogLevel level = LOG_INFO);

void set_progress_bar_detailed(bool detailed);

} 
} 

#endif
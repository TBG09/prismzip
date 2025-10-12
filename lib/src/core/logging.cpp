#include <prism/core/logging.h>

namespace prism {
namespace core {

static LogHandler g_log_handler = nullptr;

void set_log_handler(LogHandler handler) {
    g_log_handler = handler;
}

void log(const std::string& msg, LogLevel level) {
    if (g_log_handler) {
        g_log_handler(msg, static_cast<int>(level));
    }
}

} // namespace core
} // namespace prism

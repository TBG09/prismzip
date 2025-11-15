#ifndef PRISM_CORE_UI_UTILS_H
#define PRISM_CORE_UI_UTILS_H

#include <string>
#include <cstdint>

namespace prism {
namespace core {

void show_progress_bar(int current, int total, const std::string& file_path, uint64_t file_size, bool raw_output = false, bool use_basic_chars = false);
bool confirm_action(const std::string& message, bool auto_yes);

}
}

#endif 

#ifndef PRISM_CORE_UI_UTILS_H
#define PRISM_CORE_UI_UTILS_H

#include <string>

namespace prism {
namespace core {

void show_progress_bar(int current, int total, int width = 40);
bool confirm_action(const std::string& message, bool auto_yes = false);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_UI_UTILS_H

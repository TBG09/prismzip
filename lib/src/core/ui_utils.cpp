#include <prism/core/ui_utils.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm> // Required for std::transform

namespace prism {
namespace core {

void show_progress_bar(int current, int total, int width) {
    if (total == 0) return;
    
    float percent = (float)current / total * 100;
    int filled = width * current / total;
    
    std::string bar = "";
    for(int i = 0; i < filled; i++) bar += "█";
    for(int i = 0; i < width - filled; i++) bar += "░";
    
    // This still prints to cout, which is a side effect. 
    // A better library would use a progress callback.
    std::cout << "\rProgress: [" << bar << "] " 
         << std::fixed << std::setprecision(1) << percent << "% (" 
         << current << "/" << total << ")" << std::flush;
}

bool confirm_action(const std::string& message, bool auto_yes) {
    if (auto_yes) {
        std::cout << message << " [Y/n] Y (auto-confirmed)" << std::endl;
        return true;
    }

    std::string response;
    while (true) {
        std::cout << message << " [Y/n] ";
        std::getline(std::cin, response);
        std::transform(response.begin(), response.end(), response.begin(), ::tolower);

        if (response == "y" || response == "yes" || response == "") { // Default to yes
            return true;
        } else if (response == "n" || response == "no") {
            return false;
        } else {
            std::cout << "Invalid input. Please enter 'y' or 'n'." << std::endl;
        }
    }
}

} // namespace core
} // namespace prism


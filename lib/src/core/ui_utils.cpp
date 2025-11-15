#include <prism/core/ui_utils.h>
#include <prism/core/file_utils.h>
#include <iostream>
#include <iomanip>

namespace prism {
namespace core {

void show_progress_bar(int current, int total, const std::string& file_path, uint64_t file_size, bool raw_output, bool use_basic_chars) {
    if (raw_output) {
        std::cout << current << "/" << total << "\r";
        if (current == total) {
            std::cout << std::endl;
        }
    } else {
        const int bar_width = 30;
        float progress = (float)current / total;
        int pos = bar_width * progress;

        std::cout << "[";
        if (use_basic_chars) {
            for (int i = 0; i < bar_width; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
        } else {
            for (int i = 0; i < bar_width; ++i) {
                if (i < pos) std::cout << "█";
                else std::cout << "░";
            }
        }
        std::cout << "] " << std::fixed << std::setprecision(1) << progress * 100.0 << "% ("
                  << current << "/" << total << ") - Added file '" << file_path 
                  << "' (" << format_size(file_size) << ")\r";
        
        std::cout.flush();
        if (current == total) {
            std::cout << std::endl;
        }
    }
}

bool confirm_action(const std::string& message, bool auto_yes) {
    if (auto_yes) {
        return true;
    }
    
    std::cout << message << " (y/n): ";
    char response;
    std::cin >> response;
    return (response == 'y' || response == 'Y');
}

}
}

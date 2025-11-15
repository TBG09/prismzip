#include <prism/core/ui_utils.h>
#include <prism/core/file_utils.h>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace prism {
namespace core {

static bool g_detailed_progress_bar = false;

void set_progress_bar_detailed(bool detailed) {
    g_detailed_progress_bar = detailed;
}

void show_progress_bar(int current, int total, const std::string& file_path, uint64_t file_size, uint64_t compressed_size, std::chrono::steady_clock::time_point start_time, bool raw_output, bool use_basic_chars) {
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
                  << "' (" << format_size(file_size) << ")";

        if (g_detailed_progress_bar) {
            double ratio = 0;
            if (file_size > 0) {
                ratio = 100.0 * (1.0 - (double)compressed_size / file_size);
            }
            std::cout << " (Ratio: " << std::fixed << std::setprecision(1) << ratio << "%)";

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            if (current > 0 && elapsed > 0) {
                double time_per_file = (double)elapsed / current;
                double remaining_time = time_per_file * (total - current) / 1000.0;
                std::cout << " (ETA: " << std::fixed << std::setprecision(1) << remaining_time << "s)";
            }
        }
        std::cout << "\r";
        
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

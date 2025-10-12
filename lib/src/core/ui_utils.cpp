#include "ui_utils.h"
#include <iostream>
#include <iomanip>
#include <string>

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

} // namespace core
} // namespace prism


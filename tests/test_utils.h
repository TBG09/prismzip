
#ifndef PRISM_TEST_UTILS_H
#define PRISM_TEST_UTILS_H

#include <string>
#include <vector>

// Helper function to convert string to vector<char>
inline std::vector<char> to_char_vector(const std::string& s) {
    return std::vector<char>(s.begin(), s.end());
}

// Helper function to convert vector<char> to string
inline std::string to_string(const std::vector<char>& v) {
    return std::string(v.begin(), v.end());
}

#endif // PRISM_TEST_UTILS_H

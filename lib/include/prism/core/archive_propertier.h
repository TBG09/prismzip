#ifndef PRISM_CORE_ARCHIVE_PROPERTIER_H
#define PRISM_CORE_ARCHIVE_PROPERTIER_H

#include <string>
#include <vector>

namespace prism {
namespace core {

void get_properties(const std::string& archive_file, const std::string& path_in_archive, bool auto_yes);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_PROPERTIER_H

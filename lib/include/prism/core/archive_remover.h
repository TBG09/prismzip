#ifndef PRISM_CORE_ARCHIVE_REMOVER_H
#define PRISM_CORE_ARCHIVE_REMOVER_H

#include <string>
#include <vector>

namespace prism {
namespace core {

void remove_from_archive(const std::string& archive_file, const std::vector<std::string>& paths_to_remove, bool ignore_errors);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_REMOVER_H

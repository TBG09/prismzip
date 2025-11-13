#ifndef PRISM_CORE_ARCHIVE_REMOVER_H
#define PRISM_CORE_ARCHIVE_REMOVER_H

#include <string>
#include <vector>

namespace prism {
namespace core {

void remove_from_archive(const std::string& archive_file, const std::vector<std::string>& files_to_remove, bool ignore_errors, bool raw_output = false, bool use_basic_chars = false);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_REMOVER_H

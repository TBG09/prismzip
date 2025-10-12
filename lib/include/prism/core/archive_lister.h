#ifndef PRISM_CORE_ARCHIVE_LISTER_H
#define PRISM_CORE_ARCHIVE_LISTER_H

#include <string>

namespace prism {
namespace core {

void list_archive(const std::string& archive_file, bool raw_list_mode);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_LISTER_H

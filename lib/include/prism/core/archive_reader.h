#ifndef PRISM_CORE_ARCHIVE_READER_H
#define PRISM_CORE_ARCHIVE_READER_H

#include <prism/core/types.h>
#include <string>
#include <vector>

namespace prism {
namespace core {

std::vector<FileMetadata> read_archive_metadata(const std::string& archive_file);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_READER_H

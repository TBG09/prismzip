#ifndef PRISM_CORE_ARCHIVE_EXTRACTOR_H
#define PRISM_CORE_ARCHIVE_EXTRACTOR_H

#include <string>
#include <vector>

namespace prism {
namespace core {

void extract_archive(const std::string& archive_file, const std::string& output_dir, 
                     const std::vector<std::string>& files_to_extract, bool no_overwrite, bool no_verify);

} // namespace core
} // namespace prism

#endif // PRISM_CORE_ARCHIVE_EXTRACTOR_H

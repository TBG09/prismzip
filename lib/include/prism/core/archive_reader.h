#ifndef PRISM_CORE_ARCHIVE_READER_H
#define PRISM_CORE_ARCHIVE_READER_H

#include <prism/core/types.h>
#include <string>
#include <vector>

namespace prism {
namespace core {

std::vector<FileMetadata> read_archive_metadata(const std::string& archive_file);

bool is_solid_archive(const std::string& archive_file);

FileMetadata read_non_solid_file_metadata(std::ifstream& f, uint64_t& current_offset);
std::vector<FileMetadata> read_solid_block_metadata(std::ifstream& f, uint64_t& uncompressed_offset_counter, CompressionType& block_comp_type, uint8_t& block_level);

} 
} 

#endif 

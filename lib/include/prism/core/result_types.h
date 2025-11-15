#ifndef PRISM_CORE_RESULT_TYPES_H
#define PRISM_CORE_RESULT_TYPES_H

#include <cstdint>
#include <vector>

namespace prism {
namespace core {

struct ArchiveCreationResult {
    long files_added;
    uint64_t total_uncompressed_size;
    uint64_t total_compressed_size;
    uint64_t total_header_size;
    uint64_t total_metadata_size; // For solid archives, this is the metadata block size. For non-solid, it's part of total_header_size.
    uint64_t total_file_data_size;
    std::vector<long long> thread_durations_ms;
};

struct ArchiveExtractionResult {
    long files_extracted;
    long files_skipped;
    uint64_t bytes_extracted;
    int hashes_checked;
    int hash_mismatches;
    std::vector<long long> thread_durations_ms;
};

} 
} 

#endif 

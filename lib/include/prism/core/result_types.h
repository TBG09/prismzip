#ifndef PRISM_CORE_RESULT_TYPES_H
#define PRISM_CORE_RESULT_TYPES_H

#include <cstdint>
#include <vector>

namespace prism {
namespace core {

struct ArchiveCreationResult {
    int files_added;
    uint64_t total_uncompressed_size;
    uint64_t total_compressed_size;
    std::vector<long long> thread_durations_ms;
};

struct ArchiveExtractionResult {
    int files_extracted;
    int files_skipped;
    uint64_t bytes_extracted;
    int hashes_checked;
    int hash_mismatches;
    std::vector<long long> thread_durations_ms;
};

} 
} 

#endif 

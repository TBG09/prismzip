#include <prism/core/archive_reader.h>
#include <prism/core/logging.h>
#include <fstream>
#include <cstring>

namespace prism {
namespace core {

std::vector<FileMetadata> read_archive_metadata(const std::string& archive_file) {
    std::vector<FileMetadata> items;
    
    std::ifstream f(archive_file, std::ios::binary);
    if (!f) {
        log("Error: Archive file not found: '" + archive_file + "'", LOG_ERROR);
        // The original code called exit(1). In a library, we should throw an exception.
        throw std::runtime_error("Archive file not found: " + archive_file);
    }
    
    log("Reading archive metadata from '" + archive_file + "'...", LOG_VERBOSE);
    
    char magic[4];
    uint16_t version;
    f.read(magic, 4);
    f.read((char*)&version, 2);
    
    if (strncmp(magic, "PRZM", 4) != 0 || version != 1) {
        log("Error: Invalid archive format.", LOG_ERROR);
        throw std::runtime_error("Invalid archive format.");
    }
    
    while (f.peek() != EOF) {
        FileMetadata item;
        item.header_start_offset = f.tellg();
        
        uint32_t path_len;
        f.read((char*)&path_len, 4);
        if (f.eof()) break;
        
        item.path.resize(path_len);
        f.read(&item.path[0], path_len);
        
        uint8_t comp_type, level, hash_type_val;
        f.read((char*)&comp_type, 1);
        f.read((char*)&level, 1);
        f.read((char*)&hash_type_val, 1);
        item.compression_type = static_cast<CompressionType>(comp_type);
        item.level = level;
        item.hash_type = static_cast<HashType>(hash_type_val);
        
        uint16_t hash_len;
        f.read((char*)&hash_len, 2);
        
        item.file_hash.resize(hash_len);
        f.read(&item.file_hash[0], hash_len);
        
        f.read((char*)&item.file_size, 8);
        f.read((char*)&item.compressed_size, 8);
        
        item.data_start_offset = f.tellg();
        items.push_back(item);
        
        f.seekg(item.compressed_size, std::ios::cur);
    }
    
    log("Finished reading archive metadata.", LOG_VERBOSE);
    return items;
}

} // namespace core
} // namespace prism

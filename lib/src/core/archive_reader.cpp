#include <prism/core/archive_reader.h>
#include <prism/core/logging.h>
#include <fstream>
#include <cstring>

namespace prism {
namespace core {

FileMetadata read_non_solid_file_metadata(std::ifstream& f, uint64_t& current_offset) {
    FileMetadata item;
    item.header_start_offset = current_offset;
    
    uint32_t path_len;
    f.read((char*)&path_len, 4);
    if (f.eof()) throw std::runtime_error("Unexpected EOF while reading path length.");
    
    item.path.resize(path_len);
    f.read(&item.path[0], path_len);
    if (f.gcount() < path_len) throw std::runtime_error("Unexpected EOF while reading path.");
    
    uint8_t comp_level_hash_bytes[3];
    f.read((char*)comp_level_hash_bytes, 3);
    if (f.gcount() < 3) throw std::runtime_error("Unexpected EOF while reading compression/hash info.");

    item.compression_type = static_cast<CompressionType>(comp_level_hash_bytes[0]);
    item.level = comp_level_hash_bytes[1];
    item.hash_type = static_cast<HashType>(comp_level_hash_bytes[2]);
    
    uint16_t hash_len;
    f.read((char*)&hash_len, 2);
    if (f.gcount() < 2) throw std::runtime_error("Unexpected EOF while reading hash length.");
    
    item.file_hash.resize(hash_len);
    f.read(&item.file_hash[0], hash_len);
    if (f.gcount() < hash_len) throw std::runtime_error("Unexpected EOF while reading hash.");
    
    f.read((char*)&item.file_size, 8);
    if (f.gcount() < 8) throw std::runtime_error("Unexpected EOF while reading file size.");
    f.read((char*)&item.compressed_size, 8);
    if (f.gcount() < 8) throw std::runtime_error("Unexpected EOF while reading compressed size.");
    
    f.read((char*)&item.creation_time, 8);
    if (f.gcount() < 8) throw std::runtime_error("Unexpected EOF while reading creation time.");
    f.read((char*)&item.modification_time, 8);
    if (f.gcount() < 8) throw std::runtime_error("Unexpected EOF while reading modification time.");
    f.read((char*)&item.permissions, 4);
    if (f.gcount() < 4) throw std::runtime_error("Unexpected EOF while reading permissions.");
    f.read((char*)&item.uid, 4);
    if (f.gcount() < 4) throw std::runtime_error("Unexpected EOF while reading uid.");
    f.read((char*)&item.gid, 4);
    if (f.gcount() < 4) throw std::runtime_error("Unexpected EOF while reading gid.");
    
    item.data_start_offset = f.tellg();
    f.seekg(item.compressed_size, std::ios::cur);
    current_offset = f.tellg();
    
    return item;
}

std::vector<FileMetadata> read_solid_block_metadata(std::ifstream& f, uint64_t& uncompressed_offset_counter, CompressionType& block_comp_type, uint8_t& block_level, uint64_t& compressed_block_size) {
    std::vector<FileMetadata> block_items;

    uint64_t metadata_size;
    f.read((char*)&metadata_size, 8);
    if (f.gcount() < 8) throw std::runtime_error("Unexpected EOF while reading solid block metadata size.");
    
    std::vector<char> metadata_buffer(metadata_size);
    f.read(metadata_buffer.data(), metadata_size);
    if (f.gcount() < metadata_size) throw std::runtime_error("Unexpected EOF while reading solid block metadata.");
    
    uint64_t current_data_start_pos = f.tellg();

    f.seekg(0, std::ios::end);
    uint64_t end_of_file = f.tellg();
    f.seekg(current_data_start_pos);

    compressed_block_size = end_of_file - current_data_start_pos;

    size_t buffer_pos = 0;
    while (buffer_pos < metadata_size) {
        FileMetadata item;
        
        uint32_t path_len;
        memcpy(&path_len, &metadata_buffer[buffer_pos], 4);
        buffer_pos += 4;
        item.path.assign(&metadata_buffer[buffer_pos], path_len);
        buffer_pos += path_len;
        
        uint8_t hash_type_val;
        memcpy(&hash_type_val, &metadata_buffer[buffer_pos], 1);
        buffer_pos += 1;
        item.hash_type = static_cast<HashType>(hash_type_val);
        
        uint16_t hash_len;
        memcpy(&hash_len, &metadata_buffer[buffer_pos], 2);
        buffer_pos += 2;
        item.file_hash.assign(&metadata_buffer[buffer_pos], hash_len);
        buffer_pos += hash_len;
        
        memcpy(&item.file_size, &metadata_buffer[buffer_pos], 8);
        buffer_pos += 8;

        memcpy(&item.creation_time, &metadata_buffer[buffer_pos], 8);
        buffer_pos += 8;
        memcpy(&item.modification_time, &metadata_buffer[buffer_pos], 8);
        buffer_pos += 8;
        memcpy(&item.permissions, &metadata_buffer[buffer_pos], 4);
        buffer_pos += 4;
        memcpy(&item.uid, &metadata_buffer[buffer_pos], 4);
        buffer_pos += 4;
        memcpy(&item.gid, &metadata_buffer[buffer_pos], 4);
        buffer_pos += 4;
        
        item.compression_type = block_comp_type;
        item.level = block_level;
        item.header_start_offset = current_data_start_pos;
        item.data_start_offset = uncompressed_offset_counter;
        item.compressed_size = compressed_block_size;
        
        uncompressed_offset_counter += item.file_size;
        block_items.push_back(item);
    }
    
    f.seekg(current_data_start_pos + compressed_block_size);
    
    return block_items;
}

std::vector<FileMetadata> read_archive_metadata(const std::string& archive_file) {
    std::vector<FileMetadata> items;
    
    std::ifstream f(archive_file, std::ios::binary);
    if (!f) {
        log("Error: Archive file not found: '" + archive_file + "'", LOG_ERROR);
        throw std::runtime_error("Archive file not found: " + archive_file);
    }
    
    log("Reading archive metadata from '" + archive_file + "'...", LOG_VERBOSE);
    
    char magic[4];
    uint16_t version;
    uint8_t flags;
    f.read(magic, 4);
    f.read((char*)&version, 2);
    
    if (strncmp(magic, "PRZM", 4) != 0 || version != 1) {
        log("Error: Invalid archive format.", LOG_ERROR);
        throw std::runtime_error("Invalid archive format.");
    }
    
    f.read((char*)&flags, 1);

    uint64_t current_file_offset = f.tellg();
    uint64_t uncompressed_offset_counter = 0;

    if ((flags & SOLID_ARCHIVE_FLAG) != 0) {
        log("Reading initial solid block...", LOG_VERBOSE);
        uint8_t comp_type_val, level_val;
        f.read((char*)&comp_type_val, 1);
        f.read((char*)&level_val, 1);
        CompressionType block_comp_type = static_cast<CompressionType>(comp_type_val);
        uint8_t block_level = level_val;

        uint64_t compressed_block_size;
        std::vector<FileMetadata> block_items = read_solid_block_metadata(f, uncompressed_offset_counter, block_comp_type, block_level, compressed_block_size);
        items.insert(items.end(), block_items.begin(), block_items.end());
        current_file_offset = f.tellg();
    } else {
        log("Reading non-solid archive...", LOG_VERBOSE);
        while (f.peek() != EOF) {
            items.push_back(read_non_solid_file_metadata(f, current_file_offset));
        }
    }

    while (f.peek() != EOF) {
        char block_magic[4];
        f.read(block_magic, 4);
        if (f.gcount() < 4) break;

        if (strncmp(block_magic, SOLID_BLOCK_MAGIC, 4) == 0) {
            log("Reading appended solid block...", LOG_VERBOSE);
            uint8_t comp_type_val, level_val;
            f.read((char*)&comp_type_val, 1);
            f.read((char*)&level_val, 1);
            CompressionType block_comp_type = static_cast<CompressionType>(comp_type_val);
            uint8_t block_level = level_val;

            uint64_t compressed_block_size;
            std::vector<FileMetadata> block_items = read_solid_block_metadata(f, uncompressed_offset_counter, block_comp_type, block_level, compressed_block_size);
            items.insert(items.end(), block_items.begin(), block_items.end());
            current_file_offset = f.tellg();
        } else {
            throw std::runtime_error("Corrupted archive: unexpected block type found after initial block.");
        }
    }
    
    log("Finished reading archive metadata.", LOG_VERBOSE);
    return items;
}

bool is_solid_archive(const std::string& archive_file) {
    std::ifstream f(archive_file, std::ios::binary);
    if (!f) {
        return false;
    }
    
    char magic[4];
    uint16_t version;
    uint8_t flags;
    
    f.read(magic, 4);
    if (f.gcount() < 4) return false;

    f.read((char*)&version, 2);
    if (f.gcount() < 2) return false;
    
    if (strncmp(magic, "PRZM", 4) != 0 || version != 1) {
        return false;
    }
    
    f.read((char*)&flags, 1);
    if (f.gcount() < 1) return false;
    
    return (flags & SOLID_ARCHIVE_FLAG) != 0;
}

} // namespace core
} // namespace prism

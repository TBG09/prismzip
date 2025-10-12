#include <prism/core/types.h>

namespace prism {
namespace core {

const std::map<std::string, CompressionType> COMPRESSION_MAP = {
    {"none", CompressionType::NONE}, {"zlib", CompressionType::ZLIB}, {"bzip2", CompressionType::BZIP2}, 
    {"lzma", CompressionType::LZMA}, {"gzip", CompressionType::GZIP}, {"lz4", CompressionType::LZ4},
    {"zstd", CompressionType::ZSTD}, {"brotli", CompressionType::BROTLI}
};

const std::map<CompressionType, std::string> COMPRESSION_NAMES = {
    {CompressionType::NONE, "none"}, {CompressionType::ZLIB, "zlib"}, {CompressionType::BZIP2, "bzip2"},
    {CompressionType::LZMA, "lzma"}, {CompressionType::GZIP, "gzip"}, {CompressionType::LZ4, "lz4"},
    {CompressionType::ZSTD, "zstd"}, {CompressionType::BROTLI, "brotli"}
};

const std::map<std::string, HashType> HASH_MAP = {
    {"none", HashType::NONE}, {"md5", HashType::MD5}, {"sha1", HashType::SHA1},
    {"sha256", HashType::SHA256}, {"sha512", HashType::SHA512}, 
    {"sha384", HashType::SHA384}, {"blake2b", HashType::BLAKE2B},
    {"blake2s", HashType::BLAKE2S}, {"sha3-256", HashType::SHA3_256},
    {"sha3-512", HashType::SHA3_512}, {"ripemd160", HashType::RIPEMD160},
    {"whirlpool", HashType::WHIRLPOOL}
};

const std::map<HashType, std::string> HASH_NAMES = {
    {HashType::NONE, "none"}, {HashType::MD5, "md5"}, {HashType::SHA1, "sha1"},
    {HashType::SHA256, "sha256"}, {HashType::SHA512, "sha512"},
    {HashType::SHA384, "sha384"}, {HashType::BLAKE2B, "blake2b"},
    {HashType::BLAKE2S, "blake2s"}, {HashType::SHA3_256, "sha3-256"},
    {HashType::SHA3_512, "sha3-512"}, {HashType::RIPEMD160, "ripemd160"},
    {HashType::WHIRLPOOL, "whirlpool"}
};

const std::set<std::string> COMPRESSED_EXTENSIONS = {
    ".zip", ".rar", ".7z", ".tar.gz", ".tgz", ".tar.bz2", ".tbz2", ".tar.xz", ".txz",
    ".jpg", ".jpeg", ".png", ".gif", ".webp", ".bmp", ".tiff", ".ico",
    ".mp3", ".aac", ".ogg", ".flac", ".m4a", ".wma",
    ".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm", ".m4v",
    ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx",
    ".gz", ".bz2", ".xz", ".lz4", ".zst"
};

} // namespace core
} // namespace prism
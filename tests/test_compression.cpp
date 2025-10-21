
#include <gtest/gtest.h>
#include <prism/compression.h>
#include <vector>
#include <string>

#include "test_utils.h"
TEST(CompressionTest, ZlibCompression) {
    std::string original_data = "This is some test data for ZLIB compression. It should be compressed and then decompressed back to its original form.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::ZLIB, 9);
    ASSERT_FALSE(compressed.empty());
    ASSERT_LT(compressed.size(), original_vec.size()); // Expect compression

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::ZLIB, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

TEST(CompressionTest, Bzip2Compression) {
    std::string original_data = "This is some test data for BZIP2 compression. It should be compressed and then decompressed back to its original form.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::BZIP2, 9);
    ASSERT_FALSE(compressed.empty());

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::BZIP2, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

TEST(CompressionTest, LzmaCompression) {
    std::string original_data = "This is some test data for LZMA compression. It should be compressed and then decompressed back to its original form.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::LZMA, 9);
    ASSERT_FALSE(compressed.empty());

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::LZMA, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

TEST(CompressionTest, Lz4Compression) {
    std::string original_data = "This is some test data for LZ4 compression. It should be compressed and then decompressed back to its original form.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::LZ4, 9);
    ASSERT_FALSE(compressed.empty());
    ASSERT_LT(compressed.size(), original_vec.size()); // Expect compression

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::LZ4, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

TEST(CompressionTest, ZstdCompression) {
    std::string original_data = "This is some test data for ZSTD compression. It should be compressed and then decompressed back to its original form.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::ZSTD, 9);
    ASSERT_FALSE(compressed.empty());
    ASSERT_LT(compressed.size(), original_vec.size()); // Expect compression

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::ZSTD, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

TEST(CompressionTest, BrotliCompression) {
    std::string original_data = "This is some test data for BROTLI compression. It should be compressed and then decompressed back to its original form.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::BROTLI, 9);
    ASSERT_FALSE(compressed.empty());
    ASSERT_LT(compressed.size(), original_vec.size()); // Expect compression

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::BROTLI, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

TEST(CompressionTest, NoCompression) {
    std::string original_data = "This data should not be compressed.";
    std::vector<char> original_vec = to_char_vector(original_data);

    std::vector<char> compressed = prism::compression::compress_data(original_vec, prism::core::CompressionType::NONE, 0);
    ASSERT_EQ(original_vec, compressed); // Expect no change

    std::vector<char> decompressed = prism::compression::decompress_data(compressed, prism::core::CompressionType::NONE, original_vec.size());
    ASSERT_EQ(original_vec, decompressed);
}

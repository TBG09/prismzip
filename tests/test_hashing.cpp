
#include <gtest/gtest.h>
#include <prism/hashing/openssl_hasher.h>
#include <vector>
#include <string>

#include "test_utils.h"
TEST(HashingTest, MD5Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "9e107d9d372bb6826bd81d3542a419d6";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::MD5);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, SHA1Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::SHA1);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, SHA256Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::SHA256);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, SHA512Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "07e547d9586f6a73f73fbac0435ed76951218fb7d0c8d788a309d785436bbb642e93a252a954f23912547d1e8a3b5ed6e1bfd7097821233fa0538f3db854fee6";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::SHA512);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, SHA224Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "730e109bd7a8a32b1cb9d9a09aa2325d2430587ddbc0c38bad911525";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::SHA224);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, SHA3_224Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "d15dadceaa4d5d7bb3b48f446421d542e08ad8887305e28d58335795";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::SHA3_224);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, SHA3_384Hash) {
    std::string data = "The quick brown fox jumps over the lazy dog";
    std::string expected_hash = "7063465e08a93bce31cd89d2e3ca8f602498696e253592ed26f07bf7e703cf328581e1471a7ba7ab119b1a9ebdf8be41";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::SHA3_384);
    EXPECT_EQ(calculated_hash, expected_hash);
}

TEST(HashingTest, NoneHash) {
    std::string data = "Some data";
    std::string expected_hash = "";
    std::string calculated_hash = prism::hashing::calculate_hash_from_data(to_char_vector(data), prism::core::HashType::NONE);
    EXPECT_EQ(calculated_hash, expected_hash);
}

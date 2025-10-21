#include <gtest/gtest.h>
#include <prism/core/archive_writer.h>
#include <prism/core/archive_reader.h>
#include <prism/core/archive_extractor.h>
#include <prism/core/archive_lister.h> 
#include <prism/core/file_utils.h>
#include <fstream>
#include <vector>

// Test fixture for archive tests
class ArchiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a dummy file for testing
        std::ofstream outfile("test_file.txt");
        outfile << "This is a test file.";
        outfile.close();
    }

    void TearDown() override {
        // Clean up generated files
        remove("test_file.txt");
        remove("test_archive.przm");
        remove("extracted_file.txt");
    }
};

TEST_F(ArchiveTest, CreateAndListArchive) {
    // 1. Create an archive
    std::vector<std::string> files_to_add = {"test_file.txt"};
    prism::core::create_archive("test_archive.przm", files_to_add, prism::core::CompressionType::NONE, 0, prism::core::HashType::NONE, false, {}, false, true);

    // 2. List the contents of the archive
    std::vector<prism::core::FileMetadata> metadata = prism::core::read_archive_metadata("test_archive.przm");

    // 3. Verify the contents
    ASSERT_EQ(metadata.size(), 1);
    EXPECT_EQ(metadata[0].path, "test_file.txt");
}

TEST_F(ArchiveTest, ExtractArchive) {
    // 1. Create an archive
    std::vector<std::string> files_to_add = {"test_file.txt"};
    prism::core::create_archive("test_archive.przm", files_to_add, prism::core::CompressionType::NONE, 0, prism::core::HashType::NONE, false, {}, false, true);

    // 2. Extract the archive
    prism::core::extract_archive("test_archive.przm", ".", {"test_file.txt"});

    // 3. Verify the extracted file
    std::ifstream extracted_file("test_file.txt");
    std::string content((std::istreambuf_iterator<char>(extracted_file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "This is a test file.");
}
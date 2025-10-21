
#include <gtest/gtest.h>
#include <prism/core/archive_writer.h>
#include <prism/core/archive_reader.h>
#include <prism/core/archive_remover.h>
#include <prism/core/file_utils.h>
#include <fstream>
#include <vector>
#include <string>

class ArchiveRemoverTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create dummy files for testing
        std::ofstream file1("file1.txt");
        file1 << "Content of file 1.";
        file1.close();

        std::ofstream file2("file2.txt");
        file2 << "Content of file 2.";
        file2.close();

        std::ofstream file3("file3.txt");
        file3 << "Content of file 3.";
        file3.close();

        // Create an archive with these files
        std::vector<std::string> files_to_add = {"file1.txt", "file2.txt", "file3.txt"};
        prism::core::create_archive("remove_test.przm", files_to_add, prism::core::CompressionType::NONE, 0, prism::core::HashType::NONE, false, {}, false, true);
    }

    void TearDown() override {
        // Clean up generated files
        remove("file1.txt");
        remove("file2.txt");
        remove("file3.txt");
        remove("remove_test.przm");
    }
};

TEST_F(ArchiveRemoverTest, RemoveSingleFile) {
    std::vector<std::string> to_remove = {"file2.txt"};
    prism::core::remove_from_archive("remove_test.przm", to_remove, false);

    std::vector<prism::core::FileMetadata> metadata = prism::core::read_archive_metadata("remove_test.przm");
    ASSERT_EQ(metadata.size(), 2);
    EXPECT_EQ(metadata[0].path, "file1.txt");
    EXPECT_EQ(metadata[1].path, "file3.txt");
}

TEST_F(ArchiveRemoverTest, RemoveMultipleFiles) {
    std::vector<std::string> to_remove = {"file1.txt", "file3.txt"};
    prism::core::remove_from_archive("remove_test.przm", to_remove, false);

    std::vector<prism::core::FileMetadata> metadata = prism::core::read_archive_metadata("remove_test.przm");
    ASSERT_EQ(metadata.size(), 1);
    EXPECT_EQ(metadata[0].path, "file2.txt");
}

TEST_F(ArchiveRemoverTest, RemoveNonExistentFile) {
    std::vector<std::string> to_remove = {"non_existent.txt"};
    // Expect an exception if ignore_errors is false
    ASSERT_THROW(prism::core::remove_from_archive("remove_test.przm", to_remove, false), std::runtime_error);

    // Should not throw if ignore_errors is true
    ASSERT_NO_THROW(prism::core::remove_from_archive("remove_test.przm", to_remove, true));
    std::vector<prism::core::FileMetadata> metadata = prism::core::read_archive_metadata("remove_test.przm");
    ASSERT_EQ(metadata.size(), 3); // No files should have been removed
}

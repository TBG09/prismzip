
#include <gtest/gtest.h>
#include <prism/core/file_utils.h>
#include <fstream>
#include <filesystem>
#include <algorithm> // Required for std::find

namespace fs = std::filesystem;

class FileUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create dummy files and directories for testing
        std::ofstream("test_file_exists.txt") << "content";
        fs::create_directory("test_dir_exists");
        std::ofstream("test_dir_exists/file_in_dir.txt") << "content";
        fs::create_directory("test_dir_exists/subdir");
        std::ofstream("test_dir_exists/subdir/file_in_subdir.txt") << "content";
        std::ofstream("excluded.tmp") << "content";
        std::ofstream("included.txt") << "content";
    }

    void TearDown() override {
        // Clean up generated files and directories
        fs::remove_all("test_file_exists.txt");
        fs::remove_all("test_dir_exists");
        fs::remove_all("excluded.tmp");
        fs::remove_all("included.txt");
    }
};

TEST_F(FileUtilsTest, FileExists) {
    EXPECT_TRUE(prism::core::file_exists("test_file_exists.txt"));
    EXPECT_FALSE(prism::core::file_exists("non_existent_file.txt"));
    EXPECT_TRUE(prism::core::file_exists("test_dir_exists")); // Directory exists
}

TEST_F(FileUtilsTest, IsDirectory) {
    EXPECT_TRUE(prism::core::is_directory("test_dir_exists"));
    EXPECT_FALSE(prism::core::is_directory("test_file_exists.txt"));
    EXPECT_FALSE(prism::core::is_directory("non_existent_dir"));
}

TEST_F(FileUtilsTest, GetAbsolutePath) {
    std::string abs_path = prism::core::get_absolute_path("test_file_exists.txt");
    // On Linux, this should be the current working directory + filename
    EXPECT_EQ(abs_path.substr(abs_path.length() - std::string("test_file_exists.txt").length()), "test_file_exists.txt");
    EXPECT_NE(abs_path.find(fs::current_path().string()), std::string::npos);
}

TEST_F(FileUtilsTest, ShouldExclude) {
    std::vector<std::string> exclude_patterns = {"*.tmp", "test_dir_exists/subdir/*"};
    EXPECT_TRUE(prism::core::should_exclude("excluded.tmp", exclude_patterns));
    EXPECT_FALSE(prism::core::should_exclude("included.txt", exclude_patterns));
    EXPECT_TRUE(prism::core::should_exclude("test_dir_exists/subdir/file_in_subdir.txt", exclude_patterns));
    EXPECT_FALSE(prism::core::should_exclude("test_dir_exists/file_in_dir.txt", exclude_patterns));
}

TEST_F(FileUtilsTest, ListFilesRecursive) {
    std::vector<std::string> files;
    std::vector<std::string> exclude_patterns = {};
    prism::core::list_files_recursive("test_dir_exists", files, exclude_patterns);

    // Expecting 2 files: file_in_dir.txt and file_in_subdir.txt
    EXPECT_TRUE(std::find(files.begin(), files.end(), std::string("test_dir_exists/file_in_dir.txt")) != files.end());
    EXPECT_TRUE(std::find(files.begin(), files.end(), std::string("test_dir_exists/subdir/file_in_subdir.txt")) != files.end());

    files.clear();
    exclude_patterns = {"*.txt"}; // Exclude all .txt files
    prism::core::list_files_recursive("test_dir_exists", files, exclude_patterns);
    EXPECT_TRUE(files.empty());

    files.clear();
    exclude_patterns = {"subdir/*"}; // Exclude subdir contents
    prism::core::list_files_recursive("test_dir_exists", files, exclude_patterns);
    ASSERT_EQ(files.size(), 1);
    EXPECT_TRUE(std::find(files.begin(), files.end(), std::string("test_dir_exists/file_in_dir.txt")) != files.end());
}


#include <gtest/gtest.h>
#include <prism/core/ui_utils.h>

TEST(UiUtilsTest, ShowProgressBar) {
    // This is a simple smoke test to ensure the function runs without crashing.
    // A more advanced test could capture stdout and verify the output format.
    ASSERT_NO_THROW(prism::core::show_progress_bar(10, 100));
    ASSERT_NO_THROW(prism::core::show_progress_bar(50, 100));
    ASSERT_NO_THROW(prism::core::show_progress_bar(100, 100));
}

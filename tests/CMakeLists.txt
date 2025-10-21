cmake_minimum_required(VERSION 3.10)
project(PrismZipTests)

# Use FetchContent to download Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
  CMAKE_ARGS -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)
set(CMAKE_POLICY_DEFAULT_CMP0060 NEW)
FetchContent_MakeAvailable(googletest)

# Link against the prismzip_lib
include_directories(../lib/include)

# Find all test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create the test executable
add_executable(prismzip_tests ${TEST_SOURCES})

# Link the test executable against Google Test and your library
target_link_libraries(prismzip_tests gtest_main prismzip_lib)

# Add test using gtest_discover_tests
include(GoogleTest)
gtest_discover_tests(prismzip_tests)

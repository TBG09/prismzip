cmake_minimum_required(VERSION 3.10)

project(PrismZip VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(app)

enable_testing()
add_subdirectory(tests)

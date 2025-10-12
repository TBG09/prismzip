add_executable(prismzip src/main.cpp src/cli.cpp)

target_include_directories(prismzip PUBLIC include)

target_link_libraries(prismzip PRIVATE prismzip_lib)

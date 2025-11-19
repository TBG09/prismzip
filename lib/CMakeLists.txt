include(FetchContent)

# Find dependencies
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Manually specify paths for MinGW-w64 libraries
if(WIN32)
    set(MINGW_PREFIX /usr/x86_64-w64-mingw32)
    include_directories(${MINGW_PREFIX}/include)
    link_directories(${MINGW_PREFIX}/lib)
endif()

# Try to find blake3, and if not found, use FetchContent
find_package(blake3 QUIET)
if(NOT blake3_FOUND)
  FetchContent_Declare(
    blake3
    GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
    GIT_TAG        1.5.1 # Use a specific tag for reproducibility
  )
  FetchContent_MakeAvailable(blake3)
  set(BLAKE3_TARGET blake3) # Target name from FetchContent
else()
  set(BLAKE3_TARGET blake3::blake3) # Common target name from find_package
endif()


# Gather source files
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp" "src/**/*.cpp" "src/**/**/*.cpp")

add_library(prismzip_lib STATIC ${LIB_SOURCES})

target_include_directories(prismzip_lib PUBLIC include)

target_link_libraries(prismzip_lib
    PRIVATE
        ZLIB::ZLIB
        BZip2::BZip2
        LibLZMA::LibLZMA
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
        # Manually link MinGW-w64 libraries
        lz4
        zstd
        brotlienc
        brotlidec
        snappy
        lzo2
        xxhash
        ${BLAKE3_TARGET} # Link against the correct blake3 target
)

install(TARGETS prismzip_lib
    ARCHIVE DESTINATION lib
)

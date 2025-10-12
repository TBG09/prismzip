# Find dependencies
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LZ4 REQUIRED liblz4)
pkg_check_modules(ZSTD REQUIRED libzstd)
pkg_check_modules(BROTLIENC REQUIRED libbrotlienc)
pkg_check_modules(BROTLIDEC REQUIRED libbrotlidec)

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
        ${LZ4_LIBRARIES}
        ${ZSTD_LIBRARIES}
        ${BROTLIENC_LIBRARIES}
        ${BROTLIDEC_LIBRARIES}
)

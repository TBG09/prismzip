# PrismZip

A modular, high-performance archiving utility written in C++.

## !!! Project is in a broken state! !!!

## Structure

This project is split into two main parts:

-   `lib/`: A static library that contains all the core archiving, compression, and hashing logic.
-   `app/`: A command-line interface (CLI) that uses the `prismzip_lib`.

## Building

This project uses CMake.

### Building on Linux

To build on Linux, follow these steps:

1.  **Install Dependencies:**
    Ensure you have the necessary development libraries installed for your distribution. For Arch Linux, this would typically involve:
    ```bash
    sudo pacman -S zlib bzip2 xz openssl lz4 zstd brotli snappy lzo xxhash blake3
    ```
    (Note: `blake3` might need to be installed from AUR or built from source if not available in official repos.)

2.  **Build Steps:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```
    The `prismzip` executable will be located in the `build/app/` directory.

### Cross-Compiling for Windows (from Linux)

To cross-compile for Windows from a Linux environment (e.g., Arch Linux), you will need the MinGW-w64 toolchain and the Windows versions of the required libraries.

1.  **Install MinGW-w64 Toolchain and Dependencies:**
    On Arch Linux, you can install these using `yay` (or `paru`):
    ```bash
    yay -S mingw-w64-gcc mingw-w64-zlib mingw-w64-bzip2 mingw-w64-xz mingw-w64-openssl mingw-w64-lz4 mingw-w64-zstd mingw-w64-brotli mingw-w64-snappy mingw-w64-lzo mingw-w64-xxhash
    ```
    (Note: `blake3` will be fetched and built automatically by CMake if not found on the system.)

2.  **Create a Build Directory:**
    ```bash
    mkdir build-windows
    cd build-windows
    ```

3.  **Configure with CMake (using toolchain file):**
    ```bash
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake
    ```

4.  **Build the Project:**
    ```bash
    make
    ```
    The `prismzip.exe` executable will be located in the `build-windows/app/` directory.

### Building on Windows (using MSVC or MinGW-w64)

To build natively on Windows, you will need a C++ compiler (like MSVC from Visual Studio or MinGW-w64) and the development libraries. Using a package manager like [vcpkg](https://vcpkg.io/) is highly recommended for managing dependencies.

1.  **Install vcpkg:**
    Follow the instructions on the [vcpkg GitHub page](https://github.com/microsoft/vcpkg) to install and integrate it with CMake.

2.  **Install Dependencies via vcpkg:**
    ```bash
    vcpkg install zlib bzip2 xz openssl lz4 zstd brotli snappy lzo xxhash blake3 --triplet x64-windows # Or x86-windows for 32-bit
    ```

3.  **Build Steps (using Visual Studio Command Prompt or Developer PowerShell):**
    ```bash
    mkdir build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
    cmake --build . --config Release
    ```
    The `prismzip.exe` executable will be located in the `build/Release/` (for MSVC) or `build/app/` (for MinGW-w64) directory.

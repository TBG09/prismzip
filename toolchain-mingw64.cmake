# toolchain-mingw64.cmake
# This file is used by CMake to cross-compile for MinGW-w64 (Windows) from Linux.

# Specify the target system
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64) # Or i686 for 32-bit

# Specify the cross compilers
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32) # Adjust for 32-bit if needed

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER  ${TOOLCHAIN_PREFIX}-windres) # Resource compiler

# Define where the MinGW-w64 toolchain is installed.
set(MINGW_PREFIX /usr/${TOOLCHAIN_PREFIX})

# Set the root path for searching for libraries and headers
set(CMAKE_FIND_ROOT_PATH ${MINGW_PREFIX})

# Add the MinGW-w64 prefix to CMAKE_PREFIX_PATH to help find_package
list(APPEND CMAKE_PREFIX_PATH ${MINGW_PREFIX})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

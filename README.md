# PrismZip

A modular, high-performance archiving utility written in C++.

## Structure

This project is split into two main parts:

-   `lib/`: A static library that contains all the core archiving, compression, and hashing logic.
-   `app/`: A command-line interface (CLI) that uses the `prismzip_lib`.

## Building

This project uses CMake. To build, follow these steps:

```bash
mkdir build
cd build
cmake ..
make
```

The `prismzip` executable will be located in the `build/app/` directory.

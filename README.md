# Multithreaded File System Simulator

A user-space file system simulator that models core OS-level file operations with thread-safe concurrent access, built for CS 445/445G (Operating Systems II).

## Prerequisites

- **Compiler:** A C++17 compatible compiler with pthread support (e.g., `g++`, `clang++`)
- **Platform:** Windows, Linux, or macOS

## Building

From the `Multithreaded_FS/` directory, run:

```bash
build.bat
```

This compiles all source files with `-std=c++17 -pthread` and produces `fs_test.exe`. Build output is logged to `build_log.txt`.

If you prefer to compile manually:

```bash
g++ -std=c++17 -pthread -static -g \
  -IFileControlBlock -IOpenFileTable -ISimulatedStorage \
  -IVolumeControlBlock -IDirectory -IFileSystem \
  main.cpp FileSystem/FileSystem.cpp Directory/Directory.cpp \
  FileControlBlock/FCB.cpp OpenFileTable/swOFT.cpp OpenFileTable/ptOFT.cpp \
  SimulatedStorage/SS.cpp VolumeControlBlock/VCB.cpp \
  -o fs_test.exe
```

## Running

After a successful build:

```bash
./fs_test.exe
```

This runs the multi-threaded driver program that exercises concurrent file operations (create, open, read, write, close, rename, delete) and validates synchronization correctness.

## Project Structure

```
Multithreaded_FS/
├── main.cpp                 # Driver program with concurrent test scenarios
├── FileSystem/              # Top-level FileSystem class (orchestrates all components)
├── Directory/               # Flat directory structure (file name → FCB mapping)
├── FileControlBlock/        # FCB metadata (file size, start block)
├── OpenFileTable/
│   ├── swOFT.*              # System-wide open file table
│   └── ptOFT.*              # Per-thread open file table
├── SimulatedStorage/        # 1MB simulated disk (512 × 2KB blocks)
├── VolumeControlBlock/      # Volume metadata and free-block bitmap
├── build.bat                # Build script
└── README.md
```
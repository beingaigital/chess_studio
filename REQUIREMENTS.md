# Chess Studio Requirements

This guide lists the toolchain, libraries and system packages you need before
building or packaging Chess Studio 1.0.

## Toolchain

- CMake ≥ 3.18
- Ninja ≥ 1.10 or GNU Make ≥ 4.2
- Python ≥ 3.9 (release tooling)
- Git ≥ 2.30 (version tagging & packaging)

## Compilers

- macOS: Xcode Command Line Tools (Clang 15+) or Xcode 15+
- Linux: GCC 11+ or Clang 14+
- Windows: MSVC 2022 (17.8+) with Desktop development with C++

Enable C++17 support if your toolchain does not default to it.

## Qt Dependencies

Install Qt ≥ 5.15 with the following modules:

- Qt Widgets
- Qt SVG
- Qt Concurrent
- Qt PrintSupport
- Qt Test (optional, for unit tests)

Qt 6.5+ is also supported. Ensure `qmake`, `Qt6_DIR` or `Qt5_DIR`, and `Qt Creator`
are configured if you want IDE integration.

## Platform Packages

### macOS

```bash
brew install cmake ninja qt@6
sudo xcode-select --install
```

If you plan to notarize the app, install Xcode 15+ and configure your
Developer ID certificates.

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build \
    qtbase5-dev qtdeclarative5-dev qttools5-dev-tools \
    libqt5svg5-dev libqt5concurrent5 libqt5printsupport5 \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools git python3
```

### Windows (PowerShell)

1. Install [Visual Studio 2022] with Desktop C++ workload
2. Install [Qt 6.5+] via the Qt Online Installer (add MSVC 2022 kit)
3. Install [CMake](https://cmake.org/download/) and ensure it is on `PATH`
4. Install [Ninja](https://github.com/ninja-build/ninja/releases)

## Optional Components

- Stockfish engine binaries for bundled analysis (`stockfish/` already ships sources)
- `lcov`, `gcovr` or similar coverage tools if you perform CI
- `doxygen` for regenerating API docs

## Environment Variables

- `Qt6_DIR` or `Qt5_DIR` pointing to your Qt installation
- `CMAKE_PREFIX_PATH` should include your Qt lib/cmake directory

## Quick Validation

After installing everything, verify the toolchain:

```bash
cmake --version
qmake --version
clang++ --version   # or g++/cl.exe
```

Only proceed with building Chess Studio after these commands succeed.



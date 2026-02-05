# Container Qt6 Toolchain Guide

## 1) Install toolchain (Ubuntu)
```bash
./scripts/setup_qt6_toolchain_ubuntu.sh
```

Install target:
- `cmake`, `ninja-build`, `build-essential`
- `qt6-base-dev`, `qt6-base-dev-tools`

## 2) Verify installation and build
```bash
./scripts/verify_qt6_toolchain.sh
```

Verification steps inside script:
1. Check `cmake` and `qmake6` availability.
2. Configure CMake project with Ninja generator.
3. Build the executable target.

## 3) If your environment blocks apt/network
If package repositories are inaccessible (e.g. proxy 403), run the same steps in a network-enabled Ubuntu environment, or install Qt SDK manually and set:
- `CMAKE_PREFIX_PATH=<Qt6 install>/lib/cmake`
- or `Qt6_DIR=<Qt6 install>/lib/cmake/Qt6`

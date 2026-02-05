#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

printf 'Checking essential commands...\n'
command -v cmake >/dev/null
printf '  - cmake: %s\n' "$(cmake --version | head -n1)"

if command -v qmake6 >/dev/null; then
  printf '  - qmake6: %s\n' "$(qmake6 --version | tr '\n' ' ' | sed 's/  */ /g')"
else
  printf '  - qmake6: missing\n'
fi

printf 'Configuring project...\n'
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja

printf 'Building project...\n'
cmake --build "${BUILD_DIR}" --parallel

printf '[ok] Qt6 toolchain and project build look good.\n'

#!/usr/bin/env bash
set -euo pipefail

if [[ "${EUID}" -ne 0 ]]; then
  SUDO="sudo"
else
  SUDO=""
fi

# Keep the setup minimal and explicit for Ubuntu 24.04+.
${SUDO} apt-get update
${SUDO} apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  qt6-base-dev \
  qt6-base-dev-tools

echo "[ok] Qt6 toolchain packages installed."

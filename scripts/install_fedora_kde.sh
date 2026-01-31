#!/usr/bin/env bash
set -euo pipefail

mode="runtime"
if [[ ${1:-} == "--dev" ]]; then
  mode="dev"
fi

if ! command -v dnf >/dev/null 2>&1; then
  echo "dnf not found. This script is for Fedora." >&2
  exit 1
fi

filter_available() {
  local -a input=("$@")
  local -a output=()
  local -a missing=()
  for pkg in "${input[@]}"; do
    if dnf -q repoquery --available "$pkg" >/dev/null 2>&1; then
      output+=("$pkg")
    else
      missing+=("$pkg")
    fi
  done
  if ((${#missing[@]})); then
    echo "Note: not found in repos: ${missing[*]}" >&2
  fi
  printf '%s\n' "${output[@]}"
}

runtime_pkgs=(
  qt6-qtbase
  qt6-qtbase-gui
  qt6-qtdeclarative
  qt6-qtquickcontrols2
  qt6-qtquickdialogs
  qt6-qtsvg
  qt6-qtimageformats
  qt6-qtspeech
  sqlite
  zlib
  libarchive
  poppler-qt6
  djvulibre
  speech-dispatcher
  espeak-ng
)

optional_pkgs=(
  unrar
  unar
  p7zip
  p7zip-plugins
)

dev_pkgs=(
  gcc-c++
  cmake
  ninja-build
  make
  pkgconf-pkg-config
  qt6-qtbase-devel
  qt6-qtdeclarative-devel
  qt6-qtquickcontrols2-devel
  qt6-qtspeech-devel
  qt6-qtsvg-devel
  qt6-qtimageformats-devel
  sqlite-devel
  zlib-devel
  libarchive-devel
  poppler-qt6-devel
  djvulibre-devel
)

mapfile -t install_pkgs < <(filter_available "${runtime_pkgs[@]}")

if ((${#install_pkgs[@]})); then
  echo "Installing runtime dependencies..."
  sudo dnf install -y "${install_pkgs[@]}"
fi

mapfile -t opt_pkgs < <(filter_available "${optional_pkgs[@]}")
if ((${#opt_pkgs[@]})); then
  echo "Installing optional tools (comic extraction helpers)..."
  sudo dnf install -y "${opt_pkgs[@]}" || true
fi

if [[ $mode == "dev" ]]; then
  mapfile -t build_pkgs < <(filter_available "${dev_pkgs[@]}")
  if ((${#build_pkgs[@]})); then
    echo "Installing build dependencies..."
    sudo dnf install -y "${build_pkgs[@]}"
  fi
fi

echo "Done."
if ! command -v unrar >/dev/null 2>&1; then
  echo "Note: 'unrar' may require RPM Fusion on Fedora." >&2
fi

#!/bin/sh
set -eu

source_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_dir="${XDG_CACHE_HOME:-$HOME/.cache}/river-control-center-build"

cmake -S "$source_dir" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build "$build_dir"
cmake --install "$build_dir"

printf 'Installed %s\n' "$HOME/.local/bin/river-control-center"

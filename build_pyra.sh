#!/bin/sh
set -eu
cd "$(dirname "$0")"
[ -f lib/SDL/CMakeLists.txt ] || ./prepare_pyra_dependencies.sh
cmake -S . -B build-pyra \
    -DCMAKE_BUILD_TYPE=Release \
    -DPYRA_KEEP_DEBUG_SYMBOLS=OFF
cmake --build build-pyra -j2
mkdir -p build-pyra/pvr-local
cp -f pvr-local/libpvrDRI3WSEGL.so build-pyra/pvr-local/libpvrDRI3WSEGL.so

#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PVR_SYSTEM_LIB=/opt/omap5-sgx-ddk-um-linux/lib
PVR_BUILD_LOCAL="$SCRIPT_DIR/build-pyra/pvr-local"
PVR_PROJECT_LOCAL="$SCRIPT_DIR/pvr-local"

# Prefer the rebuilt project-local DRI3 WSEGL module for OpenJKDF2.
# build-pyra/pvr-local is first because this is the path used by the tested build.
if [ -n "${LD_LIBRARY_PATH:-}" ]; then
    export LD_LIBRARY_PATH="$PVR_BUILD_LOCAL:$PVR_PROJECT_LOCAL:$PVR_SYSTEM_LIB:$LD_LIBRARY_PATH"
else
    export LD_LIBRARY_PATH="$PVR_BUILD_LOCAL:$PVR_PROJECT_LOCAL:$PVR_SYSTEM_LIB"
fi

if [ -x "$SCRIPT_DIR/build-pyra/openjkdf2" ]; then
    BIN="$SCRIPT_DIR/build-pyra/openjkdf2"
elif [ -x "$SCRIPT_DIR/openjkdf2" ]; then
    BIN="$SCRIPT_DIR/openjkdf2"
else
    echo "OpenJKDF2 binary not found. Run ./build_pyra.sh first." >&2
    exit 1
fi

exec "$BIN" "$@"

#!/bin/sh
set -eu
cd "$(dirname "$0")"

clone() {
    dir="$1"
    url="$2"
    branch="$3"
    if [ ! -f "$dir/CMakeLists.txt" ]; then
        rm -rf "$dir"
        git clone --depth 1 --branch "$branch" "$url" "$dir"
    fi
}

clone lib/libpng https://github.com/glennrp/libpng.git v1.6.39
clone lib/zlib https://github.com/madler/zlib.git v1.2.13
clone lib/GameNetworkingSockets https://github.com/ValveSoftware/GameNetworkingSockets.git v1.4.1
clone lib/protobuf https://github.com/protocolbuffers/protobuf.git v21.12
clone lib/physfs https://github.com/icculus/physfs.git release-3.2.0
clone lib/openal https://github.com/kcat/openal-soft.git 1.23.1
clone lib/SDL https://github.com/libsdl-org/SDL.git release-2.26.5
clone lib/SDL_mixer https://github.com/libsdl-org/SDL_mixer.git release-2.6.3


echo "Pyra dependencies ready."

include(ExternalProject)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${PROJECT_SOURCE_DIR}/cmake_modules/toolchain_native.cmake)
endif()

add_compile_definitions(HAVE_STRUCT_TIMEVAL HAVE_CONFIG_H BUILDING_LIBCURL USE_MBEDTLS __USE_MINGW_ANSI_STDIO CURL_STATICLIB PLATFORM_CURL)

if(DEBUG_QOL_CHEATS)
    add_compile_definitions(DEBUG_QOL_CHEATS)
endif()
if(EXPERIMENTAL_FIXED_POINT)
    add_compile_definitions(EXPERIMENTAL_FIXED_POINT)
endif()

message(STATUS "Going to build zlib 1.2.13")
include(build_zlib)

message(STATUS "Going to build PhysFS 3.2.0")
include(build_physfs)
add_compile_definitions(PLATFORM_PHYSFS)

set(PROTOBUF_DEPENDS ZLIB_PYRA)
set(PROTOBUF_BUILD_PROTOC_BINARIES TRUE)
set(GAMENETWORKINGSOCKETS_DEPENDS PROTOBUF)
message(STATUS "Going to build protobuf 3.21.12")
include(build_protobuf)
message(STATUS "Going to build GameNetworkingSockets 1.4.1")
include(build_gns)
file(GLOB TARGET_GNS_SRCS ${PROJECT_SOURCE_DIR}/src/Platform/Networking/GNS/*.cpp)
list(APPEND ENGINE_SOURCE_FILES ${TARGET_GNS_SRCS})
add_compile_definitions(PLATFORM_GNS)

message(STATUS "Going to build libpng 1.6.39")
include(build_libpng)

list(APPEND ENGINE_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/external/libsmacker/smacker.c
    ${PROJECT_SOURCE_DIR}/src/external/libsmacker/smk_bitstream.c
    ${PROJECT_SOURCE_DIR}/src/external/libsmacker/smk_hufftree.c
    ${PROJECT_SOURCE_DIR}/src/external/libsmusher/src/smush.c
    ${PROJECT_SOURCE_DIR}/src/external/libsmusher/src/codec48.c
)

message(STATUS "Going to build SDL 2.26.5 (Pyra patched)")
include(build_sdl)
message(STATUS "Going to build SDL_mixer 2.6.3")
include(build_sdl_mixer)

file(GLOB TARGET_SDL2_SRCS ${PROJECT_SOURCE_DIR}/src/Platform/SDL2/*.c)
list(APPEND ENGINE_SOURCE_FILES ${TARGET_SDL2_SRCS})

file(GLOB TARGET_GL_SRCS ${PROJECT_SOURCE_DIR}/src/Platform/GL/*.c)
file(GLOB TARGET_GL_CPP_SRCS ${PROJECT_SOURCE_DIR}/src/Platform/GL/*.cpp)
list(APPEND ENGINE_SOURCE_FILES ${TARGET_GL_SRCS} ${TARGET_GL_CPP_SRCS})

message(STATUS "Going to build OpenAL Soft 1.23.1")
include(build_openal)

file(GLOB TARGET_POSIX_SRCS ${PROJECT_SOURCE_DIR}/src/Platform/Posix/*.c)
list(APPEND ENGINE_SOURCE_FILES ${TARGET_POSIX_SRCS})
list(APPEND ENGINE_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/external/nativefiledialog-extended/nfd_gtk.cpp)

# GameNetworkingSockets/protobuf runtime zlib helper, native Pyra build only.
set(HACK_ZLIB_SRC ${ZLIB_SHARED_LIBRARY_PATH})
set(HACK_ZLIB_SRC_DIR ${ZLIB_SHARED_LIBRARY_DIR})
set(GNS_PROTOC_HACK_ZLIB ${GameNetworkingSockets_ROOT}/src/.copied_hack)
set(GNS_PROTOC_HACK_ZLIB_DIR ${GameNetworkingSockets_ROOT}/src)
set(GNS_PROTOC_HACK_ZLIB_DIR_2 ${Protobuf_ROOT}/lib)
set(GNS_PROTOC_HACK_ZLIB_DIR_3 ${Protobuf_ROOT}/lib)
add_custom_command(OUTPUT "${GNS_PROTOC_HACK_ZLIB}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GNS_PROTOC_HACK_ZLIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HACK_ZLIB_SRC_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch "${HACK_ZLIB_SRC_DIR}/hack.so"
    COMMAND ${CMAKE_COMMAND} -E copy "${HACK_ZLIB_SRC_DIR}/*.so" "${GNS_PROTOC_HACK_ZLIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${HACK_ZLIB_SRC_DIR}/*.so" "${GNS_PROTOC_HACK_ZLIB_DIR_2}"
    COMMAND ${CMAKE_COMMAND} -E copy "${HACK_ZLIB_SRC_DIR}/*.so" "${GNS_PROTOC_HACK_ZLIB_DIR_3}"
    COMMAND ${CMAKE_COMMAND} -E touch "${GNS_PROTOC_HACK_ZLIB}")
add_custom_target(GNS_HACK_ZLIB DEPENDS ${GNS_PROTOC_HACK_ZLIB})
add_dependencies(GameNetworkingSockets::GameNetworkingSockets GNS_HACK_ZLIB)
add_dependencies(GameNetworkingSockets::GameNetworkingSockets_s GNS_HACK_ZLIB)
add_dependencies(GNS_HACK_ZLIB PROTOBUF)

file(GLOB EMBEDDED_RESOURCES ${PROJECT_SOURCE_DIR}/resource/ui/* ${PROJECT_SOURCE_DIR}/resource/shaders/*)

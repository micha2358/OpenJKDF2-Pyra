cmake_minimum_required(VERSION 3.20)

include(cmake_modules/version.cmake)
project(OpenJKDF2 VERSION ${OPENJKDF2_PROJECT_VERSION} LANGUAGES C CXX)

message(STATUS "OpenJKDF2 Pyra-only build")
message(STATUS "Target: DragonBox Pyra / OMAP5432 Cortex-A15 / PowerVR SGX544MP2 / GLES2")

set(BIN_NAME "openjkdf2")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# This source tree intentionally supports one target only: DragonBox Pyra.
set(TARGET_USE_CURL TRUE)
set(TARGET_USE_PHYSFS TRUE)
set(TARGET_USE_BASICSOCKETS FALSE)
set(TARGET_USE_GAMENETWORKINGSOCKETS TRUE)
set(TARGET_USE_LIBSMACKER TRUE)
set(TARGET_USE_LIBSMUSHER TRUE)
set(TARGET_USE_SDL2 TRUE)
set(TARGET_USE_OPENGL TRUE)
set(TARGET_USE_GLES2 TRUE)
set(TARGET_USE_OPENAL TRUE)
set(TARGET_POSIX TRUE)
set(TARGET_LINUX TRUE)
set(TARGET_NO_BLOBS TRUE)
set(TARGET_CAN_JKGM FALSE)
set(TARGET_NO_MULTIPLAYER_MENUS FALSE)
set(TARGET_FIND_OPENAL TRUE)
set(OPENJKDF2_USE_BLOBS FALSE CACHE BOOL "Use blobs" FORCE)

set(DEBUG_QOL_CHEATS $ENV{DEBUG_QOL_CHEATS})
set(EXPERIMENTAL_FIXED_POINT $ENV{EXPERIMENTAL_FIXED_POINT})

if(DEFINED ENV{OPENJKDF2_NO_ASAN})
    set(OPENJKDF2_NO_ASAN $ENV{OPENJKDF2_NO_ASAN})
else()
    set(OPENJKDF2_NO_ASAN TRUE)
endif()

add_compile_definitions(
    OPENJKDF2_PROJECT_NAME=${PROJECT_NAME}
    OPENJKDF2_PROJECT_VERSION=${PROJECT_VERSION}
    OPENJKDF2_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
    OPENJKDF2_MINOR_VERSION=${PROJECT_VERSION_MINOR}
    OPENJKDF2_PATCH_VERSION=${PROJECT_VERSION_PATCH}
    OPENJKDF2_TWEAK_VERSION=${PROJECT_VERSION_TWEAK}
    OPENJKDF2_RELEASE_VERSION_STRING=\"v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\"
    OPENJKDF2_RELEASE_VERSION_STRING_W=L\"v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\"
    OPENJKDF2_RELEASE_COMMIT=\"${OPENJKDF2_RELEASE_COMMIT}\"
    OPENJKDF2_RELEASE_COMMIT_SHORT=\"${OPENJKDF2_RELEASE_COMMIT_SHORT}\"
    OPENJKDF2_RELEASE_COMMIT_W=L\"${OPENJKDF2_RELEASE_COMMIT}\"
    OPENJKDF2_RELEASE_COMMIT_SHORT_W=L\"${OPENJKDF2_RELEASE_COMMIT_SHORT}\"
    QOL_IMPROVEMENTS
    TARGET_PYRA
    TARGET_GLES2
    NO_JK_MMAP
    LINUX_TMP
    LINUX
    PLATFORM_LINUX
    PLATFORM_POSIX
    PLATFORM_NO_CACERT_BLOB
    SDL2_RENDER
    STDSOUND_OPENAL
)
add_definitions(-D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE)

set(PYRA_CPU_FLAGS
    -mcpu=cortex-a15
    -mfpu=neon-vfpv4
    -mfloat-abi=hard
    -fomit-frame-pointer
)
add_compile_options(${PYRA_CPU_FLAGS} -pipe -fshort-wchar)
add_link_options(${PYRA_CPU_FLAGS} -fshort-wchar)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit-function-declaration -Wno-unused-variable -Wno-parentheses")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-parentheses")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
option(PYRA_KEEP_DEBUG_SYMBOLS "Keep minimal debug symbols in the final executable" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
elseif(PYRA_KEEP_DEBUG_SYMBOLS)
    # Final Pyra build with compact line/function information for useful backtraces.
    add_compile_options(-O3 -g1)
else()
    # Fully stripped release variant: do not emit compiler debug information.
    add_compile_options(-O3 -g0)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

# Pyra-only: nlohmann/json is header-only; keep only the single-header distribution.
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE
    ${PROJECT_SOURCE_DIR}/3rdparty/json/single_include
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/external/libsmacker
    ${PROJECT_SOURCE_DIR}/src/external/libsmusher/src
    ${PROJECT_SOURCE_DIR}/src/external/nativefiledialog-extended
)

file(GLOB ENGINE_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.c
    ${PROJECT_SOURCE_DIR}/src/*/*.c
    ${PROJECT_SOURCE_DIR}/src/General/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Platform/Common/*.c
    ${PROJECT_SOURCE_DIR}/src/Platform/Common/*.cpp
)
file(GLOB GAME_MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.c)
list(REMOVE_ITEM ENGINE_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.c)

file(GLOB FCASEOPEN_SRCS ${PROJECT_SOURCE_DIR}/src/external/fcaseopen/*.c)
list(APPEND ENGINE_SOURCE_FILES ${FCASEOPEN_SRCS})
include_directories(${PROJECT_SOURCE_DIR}/src/external/fcaseopen)

include(cmake_modules/config_platform_deps.cmake)
include(cmake_modules/precompile_globals.cmake)

file(GLOB ENGINE_SOURCE_FILES_REGLOB
    ${PROJECT_SOURCE_DIR}/src/*.c
    ${PROJECT_SOURCE_DIR}/src/*/*.c
    ${PROJECT_SOURCE_DIR}/src/General/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Platform/*/*.c
    ${PROJECT_SOURCE_DIR}/src/Platform/*/*/*.c
    ${PROJECT_SOURCE_DIR}/src/Platform/*/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Platform/*/*/*.cpp
)
list(APPEND ENGINE_SOURCE_FILES_REGLOB ${GLOBALS_C})
if(EXPERIMENTAL_FIXED_POINT)
    set_source_files_properties(${ENGINE_SOURCE_FILES_REGLOB} PROPERTIES LANGUAGE CXX)
endif()

add_library(sith_engine OBJECT ${ENGINE_SOURCE_FILES})

# SDL/OpenAL are built locally from the pinned Pyra dependency sources.
set(SDL2_COMMON_LIBS SDL2main SDL::SDL ${SDL_MIXER_DEPS} SDL::Mixer OpenAL::OpenAL)
add_executable(${BIN_NAME} ${GAME_MAIN_SOURCE_FILES})
target_link_libraries(${BIN_NAME} PRIVATE ${SDL2_COMMON_LIBS} sith_engine)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

if(TARGET_USE_CURL)
    pkg_check_modules(LIBCURL REQUIRED libcurl)
    include_directories(${LIBCURL_STATIC_INCLUDE_DIRS})
    link_directories(${LIBCURL_STATIC_LIBRARY_DIRS})
    add_definitions(${LIBCURL_STATIC_CFLAGS_OTHER})
endif()

# Direct native PowerVR EGL/GLES2.
# Do not link by the generic name "GLESv2": on Debian that may resolve to
# Mesa's /usr/lib libGLESv2 while SDL dynamically loads the PowerVR library.
# Mixing both implementations leaves the direct gl*() calls without the
# current PowerVR context.
set(PYRA_POWERVR_LIBDIR "/opt/omap5-sgx-ddk-um-linux/lib")
set(PYRA_GLES2_LIBRARY "${PYRA_POWERVR_LIBDIR}/libGLESv2.so")
set(PYRA_EGL_LIBRARY   "${PYRA_POWERVR_LIBDIR}/libEGL.so")

if(NOT EXISTS "${PYRA_GLES2_LIBRARY}")
    message(FATAL_ERROR "Pyra PowerVR GLES2 library not found: ${PYRA_GLES2_LIBRARY}")
endif()
if(NOT EXISTS "${PYRA_EGL_LIBRARY}")
    message(FATAL_ERROR "Pyra PowerVR EGL library not found: ${PYRA_EGL_LIBRARY}")
endif()

message(STATUS "Pyra PowerVR GLES2: ${PYRA_GLES2_LIBRARY}")
message(STATUS "Pyra PowerVR EGL:   ${PYRA_EGL_LIBRARY}")

target_link_libraries(sith_engine PRIVATE PNG::PNG ZLIB::ZLIB)
target_link_libraries(sith_engine PRIVATE
    ${SDL2_COMMON_LIBS}
    "${PYRA_GLES2_LIBRARY}"
    "${PYRA_EGL_LIBRARY}"
    ${OPENAL_LIBRARY}
    ${GTK3_LIBRARIES}
)

# Keep the PowerVR directory in the runtime search path as an additional
# safeguard for transitive EGL/GLES dependencies.
set_target_properties(${BIN_NAME} PROPERTIES
    BUILD_RPATH "${PYRA_POWERVR_LIBDIR}"
    INSTALL_RPATH "${PYRA_POWERVR_LIBDIR}"
)
target_link_libraries(sith_engine PRIVATE PhysFS::PhysFS_s)
target_link_libraries(${BIN_NAME} PRIVATE PhysFS::PhysFS_s)
target_link_libraries(sith_engine PRIVATE GameNetworkingSockets::GameNetworkingSockets)
target_link_libraries(sith_engine PRIVATE nlohmann_json::nlohmann_json dl curl)

add_custom_command(TARGET ${BIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_BINARY_DIR}/GameNetworkingSockets/bin/libGameNetworkingSockets.so
            ${PROJECT_BINARY_DIR}/libGameNetworkingSockets.so
    COMMENT "Copying GameNetworkingSockets runtime library")

if(NOT OPENJKDF2_NO_ASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=float-divide-by-zero")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=float-divide-by-zero")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=float-divide-by-zero")
    add_compile_options(-fsanitize=leak)
    add_link_options(-fsanitize=leak)
endif()


if(NOT PYRA_KEEP_DEBUG_SYMBOLS AND
   (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))
    add_custom_command(TARGET ${BIN_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:${BIN_NAME}>
        COMMENT "Stripping all non-dynamic symbols from ${BIN_NAME}")
endif()

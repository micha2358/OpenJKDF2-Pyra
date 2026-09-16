set(SDL_ROOT ${CMAKE_BINARY_DIR}/SDL)
set(SDL_HIDAPI TRUE)
set(SDL_STATIC TRUE)
set(SDL_SHARED FALSE)
set(SDL_FOUND TRUE)
set(SDL_INCLUDE_DIRS ${SDL_ROOT}/include/SDL2)
set(SDL_LIBRARIES ${SDL_ROOT}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX})
set(SDL_VERSION 2.26.5)

ExternalProject_Add(
    SDL
    SOURCE_DIR          ${CMAKE_SOURCE_DIR}/lib/SDL
    BINARY_DIR          ${SDL_ROOT}
    INSTALL_DIR         ${SDL_ROOT}
    UPDATE_DISCONNECTED TRUE
    CMAKE_ARGS          --toolchain ${CMAKE_TOOLCHAIN_FILE}
                        --install-prefix ${SDL_ROOT}
                        -DCMAKE_INSTALL_LIBDIR=lib
                        -DCMAKE_BUILD_TYPE:STRING=Release
                        -DSDL_SHARED:BOOL=FALSE
                        -DSDL_STATIC:BOOL=TRUE
                        -DSDL_TEST:BOOL=FALSE
                        -DSDL_HIDAPI:BOOL=TRUE
                        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    BUILD_BYPRODUCTS    ${SDL_LIBRARIES}
)

add_library(SDL::SDL STATIC IMPORTED)
add_dependencies(SDL::SDL SDL)
file(MAKE_DIRECTORY ${SDL_INCLUDE_DIRS})
set_target_properties(SDL::SDL PROPERTIES IMPORTED_LOCATION ${SDL_LIBRARIES})
target_include_directories(SDL::SDL INTERFACE ${SDL_INCLUDE_DIRS})
target_link_directories(SDL::SDL INTERFACE ${SDL_ROOT}/lib)
set(SDL2_LIBRARY ${SDL_ROOT}/lib)
set(SDL2_INCLUDE_DIR ${SDL_INCLUDE_DIRS})

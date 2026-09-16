# DragonBox Pyra build (SDL2 + OpenGL ES 2.0)

This tree adds `PLAT_LINUX_PYRA`, targeting ARMv7 Cortex-A15, SDL 2.26.5 and OpenGL ES 2.0. The GLES2 baseline disables Bloom/SSAO/MRT and uses GLSL ES 1.00-compatible shader paths.

The upstream source ZIP does not contain the Git submodule payloads. `build_pyra.sh` automatically runs `prepare_pyra_dependencies.sh` to fetch the pinned upstream dependency versions when they are missing.

On Debian/PyraOS install the normal build prerequisites plus GLES2 development headers/libraries, GTK3, curl and Git. Then run:

```sh
./build_pyra.sh
```

Equivalent manual configure:

```sh
cmake -S . -B build-pyra -DPLAT_LINUX_PYRA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-pyra -j2
```

CPU flags are set by the Pyra target: `-mcpu=cortex-a15 -mfpu=neon-vfpv4 -mfloat-abi=hard -fomit-frame-pointer -O2 -pipe`.

## Current GLES2 baseline

* SDL2 window/context requests GLES 2.0.
* GLEW/FreeGLUT are not used by the Pyra target.
* Renderer uses one color attachment and a 16-bit depth buffer.
* Bloom and SSAO are disabled on GLES2.
* Desktop OpenGL paths are retained for the other targets.
* GLSL ES 1.00 compatibility is enabled for default/menu/UI/FBO shaders.

This package was prepared structurally in a non-ARM build environment; the final compiler/runtime validation must be done on the Pyra because its PowerVR GLES2 driver and ARM hard-float toolchain are not available in the packaging environment.

## Startup SIGSEGV fix

The Pyra target defines `NO_JK_MMAP` and `LINUX_TMP`. This disables the legacy 32-bit `MAP_FIXED` JK.EXE memory mapping around address `0x400000`, which can overwrite the native ARM executable itself and cause an immediate segmentation fault before SDL2/GLES2 initialization.

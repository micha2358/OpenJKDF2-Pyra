# OpenJKDF2 0.9.9 – DragonBox Pyra only

This tree is intentionally reduced to one supported target:

- DragonBox Pyra
- OMAP5432 / dual Cortex-A15 / ARMv7 hard-float
- PowerVR SGX544MP2
- SDL2 2.26.5
- native EGL / OpenGL ES 2.0 (no desktop OpenGL/Mesa build path)

## Build

```sh
./prepare_pyra_dependencies.sh
./build_pyra.sh
```

The build uses `-mcpu=cortex-a15 -mfpu=neon-vfpv4 -mfloat-abi=hard` and links against native `GLESv2`.

## Pyra-specific runtime fixes retained

- direct PowerVR EGL/GLES2 startup path
- current working directory is the game data directory
- Pyra-safe byte write helper used by string-table copies
- Smacker/cutscene fixes from the Pyra port

The temporary joystick logging and the experimental `HandleBall()` guard are not part of this tree. The SDL `HandleBall()` implementation remains upstream SDL 2.26.5 behavior. The final Pyra build uses the system PowerVR EGL/GLES2/WSEGL stack from `/opt/omap5-sgx-ddk-um-linux/lib`; no project-local WSEGL override and no application-side GOB or SDL joystick file-descriptor relocation are used.

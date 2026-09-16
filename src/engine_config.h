#ifndef _OPENJKDF2_ENGINE_CONFIG_H
#define _OPENJKDF2_ENGINE_CONFIG_H

#include <float.h>
#include "types_enums.h"

// If I ever do demo recording, add it here
#define NEEDS_STEPPED_PHYS (!jkPlayer_bJankyPhysics || sithNet_isMulti)

// Settings for stepped physics
#define TARGET_PHYSTICK_FPS (sithNet_isMulti ? (sithNet_tickrate < 100 ? 150.0 : 50.0) : 150.0)
#define DELTA_PHYSTICK_FPS (1.0/TARGET_PHYSTICK_FPS)

// Settings for the old stepped physics
#define OLDSTEP_TARGET_FPS (sithNet_isMulti ? (sithNet_tickrate < 100 ? 150.0 : 50.0) : 50.0)
#define OLDSTEP_DELTA_50FPS (1.0/OLDSTEP_TARGET_FPS)

// Default setting for canonical COG tickrate minimum.
// In the original game, sleeps < 0.02s will always round up to 0.02s
// in practice, because every COG execute tick was always 0.02s apart from
// the frame limiter.
// For consistency on some sector thrusts (Lv18's air shafts for example)
// we have to round up.
#define CANONICAL_COG_TICKRATE (1.0 / 50.0)

// Default setting for canonical physics tickrate minimum.
// In the original game, slopes drag the player downwards
// faster depending on the framerate.
// At high FPS, this causes the player to get stuck climbing
// up slopes, because the player falls more than they were
// able to climb, creating a barrier.
//
// This default is based on the boxes in Training.jkl in Droidworks
#define CANONICAL_PHYS_TICKRATE (1.0 / 25.0)

// Use microsecond timing to calculate sithTime_deltaSecs/etc
#define MICROSECOND_TIME

// Original game will speed up if framerate is over 100?
#define SITHTIME_MINDELTA (1)
#define SITHTIME_MAXDELTA (500)
#define SITHTIME_MINDELTA_US (1)
#define SITHTIME_MAXDELTA_US (500*1000)

#define MULTI_NEXTLEVEL_DELAY_MS (2000)
#define MULTI_LEAVEJOIN_DELAY_MS (1000)
#define MULTI_TIMEOUT_MS (30000)
#define MULTI_SCORE_UPDATE_INTERVAL_MS (5000)
#define MULTI_BIG_UPDATE_INTERVAL_MS (30)

// World limits
#define SITH_MAX_THINGS (32000)
#define SITH_MAX_VISIBLE_SECTORS (1024)
#define SITH_MAX_VISIBLE_SECTORS_2 (1280)
#define SITH_MAX_VISIBLE_ALPHA_SURFACES (1024)
#define SITH_MAX_SURFACE_CLIP_ITERS (50)

// COG resource limits
#define SITHCOGVM_MAX_STACKSIZE (0x10000)
#define SITHCOG_SYMBOL_LIMIT (2048) // JK was 512, MoTS/DW are 1024
#define SITHCOG_LINKED_SYMBOL_LIMIT (2048)
#define SITHCOG_MAX_LINKS (2048)
#define SITHCOG_NODE_STACKDEPTH (0x800) // JK was 0x200, MoTS is 0x400

// Weapon-related limits
#define MAX_DEFLECTION_BOUNCES (6)

#define RDCACHE_MAX_TRIS (0x400)
#define RDCACHE_MAX_VERTICES (0x8000)

#define STD3D_MAX_TEXTURES (4096)
#define STD3D_MAX_UI_TRIS (0x8000)
#define STD3D_MAX_UI_VERTICES (0x8000)

#define SITH_MAX_SYNC_THINGS (128)

    #define SITH_NUM_EVENTS (6)

#define SITHCONTROL_NUM_HANDLERS (9)

// jkGuiMultiTally delay between maps
#define SCORE_DELAY_MS ((jkGuiNetHost_bIsDedicated && stdComm_bIsServer) ? 0 : 30000)

// UI tickrate limits
#define TICKRATE_MIN (1)
#define TICKRATE_MAX (1000)

// Run game physics at a fixed timestep
#define FIXED_TIMESTEP_PHYS

// Backport MOTS RGB lighting and bone changes
#define JKM_LIGHTING
#define JKM_BONES
#define JKM_PARAMS
#define JKM_AI
#define JKM_SABER
#define JKM_DSS
#define JKM_CAMERA

// Backport Droidworks misc
#define DW_CAMERA

#define JKPLAYER_NUM_INFOS (32)

#ifdef JKM_DSS
#define NUM_JKPLAYER_THINGS (64)
#define SITHINVENTORY_NUM_POWERKEYBINDS (32+1)
#else
#define NUM_JKPLAYER_THINGS (16)
#define SITHINVENTORY_NUM_POWERKEYBINDS (20+1)
#endif

#define SITHBIN_NUMBINS (200)

// Droidworks cameras
#ifdef DW_CAMERA
#define SITHCAMERA_NUMCAMERAS (8)
#else
#define SITHCAMERA_NUMCAMERAS (7)
#endif

#define SITHCAMERA_FOV (90.0)
#define SITHCAMERA_ASPECT (1.0)
#define SITHCAMERA_ATTENUATION_MIN (0.4)
#define SITHCAMERA_ATTENUATION_MAX (0.8)
#define SITHCAMERA_ZNEAR_FIRSTPERSON (1.0 / 128.0)
#define SITHCAMERA_ZNEAR (1.0 / 64.0)
#define SITHCAMERA_ZFAR (128.0)

#define SITHPARTICLE_MAX_PARTICLES (64)

#define RDCAMERA_MAX_LIGHTS (64)

#define STDGOB_MAX_GOBS (64)

#define RDPUPPET_MAX_TRACKS (4)
#define RDPUPPET_MAX_NODES (64)

#define JOYSTICK_MAX_STRS (6)

//
// Resource configuration
//
#define JKRES_GOB_EXT (Main_bMotsCompat ? "goo" : "gob")

#define JKRES_DF2_MAGIC_0 0x69973284
#define JKRES_DF2_MAGIC_1 0x699232C4
#define JKRES_DF2_MAGIC_2 0x69923384
#define JKRES_DF2_MAGIC_3 0x69923284

#define JKRES_MOTS_MAGIC_0 0x3B426929
#define JKRES_MOTS_MAGIC_1 0x3B426929
#define JKRES_MOTS_MAGIC_2 0x3B426929
#define JKRES_MOTS_MAGIC_3 0x3B426929

#define JKRES_MAGIC_0 (Main_bMotsCompat ? JKRES_MOTS_MAGIC_0 : JKRES_DF2_MAGIC_0)
#define JKRES_MAGIC_1 (Main_bMotsCompat ? JKRES_MOTS_MAGIC_1 : JKRES_DF2_MAGIC_1)
#define JKRES_MAGIC_2 (Main_bMotsCompat ? JKRES_MOTS_MAGIC_2 : JKRES_DF2_MAGIC_2)
#define JKRES_MAGIC_3 (Main_bMotsCompat ? JKRES_MOTS_MAGIC_3 : JKRES_DF2_MAGIC_3)

#define JKRES_IS_MOTS_MAGIC(kval) ((kval == JKRES_MOTS_MAGIC_0) || (kval == JKRES_MOTS_MAGIC_1) || (kval == JKRES_MOTS_MAGIC_2) || (kval == JKRES_MOTS_MAGIC_3))
#define JKRES_IS_DF2_MAGIC(kval) ((kval == JKRES_DF2_MAGIC_0) || (kval == JKRES_DF2_MAGIC_1) || (kval == JKRES_DF2_MAGIC_2) || (kval == JKRES_DF2_MAGIC_3))

#ifndef JKM_PARAMS
#define STDCONF_LINEBUFFER_LEN (1024)
#else
#define STDCONF_LINEBUFFER_LEN (2048)
#endif

#define SITHAI_MAX_ACTORS (256)

#define SITH_MIXER_NUMPLAYINGSOUNDS (256)

#define RDMATERIAL_MAX_TEXINFOS (16)

//
// Misc optimizations/features
//
#define SITH_DEBUG_STRUCT_NAMES


#define RDCACHE_RENDER_LINES

//#define RDMATERIAL_LRU_LOAD_UNLOAD
//#define JKGUI_SMOL_SCREEN
//#define STDHASHTABLE_CRC32_KEYS

#define JKDEV_NUM_CHEATS (64)

#define SITHCVAR_MAX_CVARS (256)
#define SITHCVAR_MAX_STRLEN (256)
#define SITHCVAR_MAX_NAME_STRLEN (64)

#define SITHCVAR_FNAME ("openjkdf2_cvars.json")
#define SITHBINDS_FNAME ("openjkdf2_binds.json")
#define REGISTRY_FNAME ("registry.json")


#define DF2_ONLY_COND(cond) ( Main_bMotsCompat || (!Main_bMotsCompat && (cond)) )
#define MOTS_ONLY_COND(cond) ( !Main_bMotsCompat || (Main_bMotsCompat && (cond)) )
#define MOTS_ONLY_FLAG(_flag) (Main_bMotsCompat ? (_flag) : (0))

#define COMPAT_SAVE_VERSION (Main_bMotsCompat ? 0x7D6 : 0x6)
#define JKSAVE_FORMATSTR (Main_bMotsCompat ? "msav%04d.jks" : "save%04d.jks")

extern int Window_isHiDpi;
#define WINDOW_DEFAULT_WIDTH  (640)
#define WINDOW_DEFAULT_HEIGHT (480)

// The type to use for flex_t:
// - float for original game behavior
// - double to verify flex_t vs flex32_t vs cog_flex_t is working
// - TODO: fixed point support?
typedef float flex_t_type; // _Float16
typedef double flex_d_t_type;

// Fixed point experiment
#ifdef EXPERIMENTAL_FIXED_POINT
#define FIXED_POINT_DECIMAL_BITS (16)
#define FIXED_POINT_WHOLE_BITS   (32-FIXED_POINT_DECIMAL_BITS)
//#define RENDER_ROUND_VERTICES
#define OPTIMIZE_AWAY_UNUSED_FIELDS
#endif

#define FLEX(n) ((flex_t)n)

// Disable warnings for Vegetable Studio
#if 1 && defined _MSC_VER
#pragma warning(disable: 4003) // not enough arguments for function-like macro invocation
#pragma warning(disable: 4005) // 'blahblah': macro redefinition
#pragma warning(disable: 4022) // pointer mismatch for actual parameter
#pragma warning(disable: 4024) // different types for formal and actual parameter
#pragma warning(disable: 4047) // 'blahblah *' differs in levels of indirection from 'blahblah (*)[blah]'
#pragma warning(disable: 4090) // different 'const' qualifiers
#pragma warning(disable: 4098) // 'void' function returning a value
#pragma warning(disable: 4113) // 'FARPROC' differs in parameter lists from 'blahblah'
#pragma warning(disable: 4133) // 'function': incompatible types - from 'blahblah [blah]' to 'const blah *'
#pragma warning(disable: 4190) // 'blahblah' has C-linkage specified, but returns UDT 'blahblahblahhhhh' which is incompatible with C
#pragma warning(disable: 4229) // anachronism used: modifiers on data are ignored
#pragma warning(disable: 4311) // 'type cast': pointer truncation from 'blah *' to 'blah'
#pragma warning(disable: 4312) // 'type cast': conversion from 'blah' to 'blah *' of greater size
#pragma warning(disable: 4700) // uninitialized local variable 'blahblah' used
#pragma warning(disable: 4715) // not all control paths return a value
#pragma warning(disable: 4716) // 'blahblah': must return a value
#pragma warning(disable: 5105) // macro expansion producing 'defined' has undefined behavior
#endif

// Optimize for math operations, depending on the architecture
// TODO: Have a TARGET_ARMvIDK or something
#define MATH_FUNC
#define FAST_FUNC
#define NO_ALIAS
#define LIKELY(cond) (cond)
#define UNLIKELY(cond) (cond)

#endif // _OPENJKDF2_ENGINE_CONFIG_H

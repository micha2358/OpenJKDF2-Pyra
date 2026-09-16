#ifndef _STDPLATFORM_H
#define _STDPLATFORM_H

#include "types.h"
#include "jk.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define stdPlatform_Startup_ADDR (0x0042C080)
#define stdPlatform_Assert_ADDR (0x0042C090)
#define stdPlatform_Printf_ADDR (0x0042C160)
#define stdPlatform_AllocHandle_ADDR (0x0042C190)
#define stdPlatform_FreeHandle_ADDR (0x0042C1A0)
#define stdPlatform_ReallocHandle_ADDR (0x0042C1B0)
#define stdPlatform_LockHandle_ADDR (0x0042C1D0)
#define stdPlatform_UnlockHandle_ADDR (0x0042C1E0)
#define stdPlatform_InitServices_ADDR (0x0042C1F0)
#define stdPlatform_GetTimeMsec_ADDR (0x0042C2B0)
#define stdPlatform_GetDateTime_ADDR (0x0042C2C0)

#define stdPlatform_GetTimeMsec_ADDR (0x0042C2B0)

void stdPlatform_InitServices(HostServices *handlers);
int stdPlatform_Startup();

// On POSIX, these are not needed — Linux_* functions handle everything
#ifndef __cplusplus
static void (*stdPlatform_Assert)(const char* a1, const char *a2, int a3) = (void*)stdPlatform_Assert_ADDR;
static void* (*stdPlatform_AllocHandle)(size_t) = (void*)stdPlatform_AllocHandle_ADDR;
static void (*stdPlatform_FreeHandle)(void*) = (void*)stdPlatform_FreeHandle_ADDR;
static void* (*stdPlatform_ReallocHandle)(void*, size_t) = (void*)stdPlatform_ReallocHandle_ADDR;
static uint32_t (*stdPlatform_LockHandle)(uint32_t) = (void*)stdPlatform_LockHandle_ADDR;
static void (*stdPlatform_UnlockHandle)(uint32_t) = (void*)stdPlatform_UnlockHandle_ADDR;
#endif

uint64_t Linux_TimeUs();
int stdPrintf(int (*a1)(const char *, ...), const char *a2, int line, const char *fmt, ...);
int stdPlatform_Printf(const char *fmt, ...);
uint32_t stdPlatform_GetTimeMsec();

int stdConsolePrintf(const char *fmt, ...);



// Added
void stdPlatform_Memzero32(void* dst, uint32_t len);
int stdPlatform_IsWordAddressableOnly(const void* p);

// Added: TWL-only extram placement hint around an allocation (no-op elsewhere).
// Use for data that is word-safe but too hot for DC's uncached VRAM window.
#define TWL_EXTRAM_SUGGEST(hs)  do {} while (0)
#define TWL_EXTRAM_RESTORE(hs)  do {} while (0)


// Added: per-file allocation cataloguing (see stdPlatform.c).
// Define STDPLATFORM_ALLOC_TRACKING to route the *_ALLOC/*_FREE macros below
// through a tracker keyed on __FILE__; without it they compile straight to the
// host-services calls (zero overhead). Free/realloc read the allocation size
// back from the platform allocator's own header, so tracked and untracked
// pointers can be mixed safely.
void* stdPlatform_TrackedAlloc(void* (*allocFn)(uint32_t), uint32_t len, const char* pFile);
void  stdPlatform_TrackedFree(void (*freeFn)(void*), void* p, const char* pFile);
void* stdPlatform_TrackedRealloc(void* (*reallocFn)(void*, uint32_t), void* p, uint32_t len, const char* pFile);
void  stdPlatform_PrintAllocStats(void);
uint32_t stdPlatform_AllocSize(void* p);

#ifdef STDPLATFORM_ALLOC_TRACKING
#define STD_ALLOC(len)       stdPlatform_TrackedAlloc(std_pHS->alloc, (len), __FILE__)
#define STD_FREE(p)          stdPlatform_TrackedFree(std_pHS->free, (p), __FILE__)
#define STD_REALLOC(p, len)  stdPlatform_TrackedRealloc(std_pHS->realloc, (p), (len), __FILE__)
#define SITH_ALLOC(len)      stdPlatform_TrackedAlloc(pSithHS->alloc, (len), __FILE__)
#define SITH_FREE(p)         stdPlatform_TrackedFree(pSithHS->free, (p), __FILE__)
#define SITH_REALLOC(p, len) stdPlatform_TrackedRealloc(pSithHS->realloc, (p), (len), __FILE__)
#define RDROID_ALLOC(len)    stdPlatform_TrackedAlloc(rdroid_pHS->alloc, (len), __FILE__)
#define RDROID_FREE(p)       stdPlatform_TrackedFree(rdroid_pHS->free, (p), __FILE__)
#define RDROID_REALLOC(p, len) stdPlatform_TrackedRealloc(rdroid_pHS->realloc, (p), (len), __FILE__)
#define JK_ALLOC(len)        stdPlatform_TrackedAlloc(pHS->alloc, (len), __FILE__)
#define JK_FREE(p)           stdPlatform_TrackedFree(pHS->free, (p), __FILE__)
#else
#define STD_ALLOC(len)       std_pHS->alloc(len)
#define STD_FREE(p)          std_pHS->free(p)
#define STD_REALLOC(p, len)  std_pHS->realloc((p), (len))
#define SITH_ALLOC(len)      pSithHS->alloc(len)
#define SITH_FREE(p)         pSithHS->free(p)
#define SITH_REALLOC(p, len) pSithHS->realloc((p), (len))
#define RDROID_ALLOC(len)    rdroid_pHS->alloc(len)
#define RDROID_FREE(p)       rdroid_pHS->free(p)
#define RDROID_REALLOC(p, len) rdroid_pHS->realloc((p), (len))
#define JK_ALLOC(len)        pHS->alloc(len)
#define JK_FREE(p)           pHS->free(p)
#endif
void stdPlatform_Memcpy32(void* dst, const void* src, uint32_t len);
void stdPlatform_Memset32(void* dst, uint8_t val, uint32_t len);

// Added: single byte store via 16-bit read-modify-write, for word-addressable
// destinations (Dreamcast VRAM / NDS slot-2 RAM drop byte-granular stores).
// Little-endian (SH4/ARM9).
static inline void stdPlatform_WriteByte16(void* p, uint8_t val)
{
    // Cortex-A15 system RAM supports byte-granular stores normally.
    // Do not use the 16-bit read-modify-write path here: at the final byte
    // of an allocation it would touch one byte beyond the valid buffer.
    *(uint8_t*)p = val;
}


#ifdef __cplusplus
}
#endif

#endif // _STDPLATFORM_H

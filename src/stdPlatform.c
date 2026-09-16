#include "stdPlatform.h"

#include "Win95/std.h"
#include "General/stdMemory.h"
#include "Main/jkQuakeConsole.h"



#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "external/fcaseopen/fcaseopen.h"

#include "SDL2_helper.h"

uint32_t Linux_TimeMs()
{
    // TWL has hardware timers we can use for accurate ms timing

    struct timespec _t;

#if defined(_MSC_VER) && !defined(WIN64_MINGW)
    timespec_get(&_t, TIME_UTC);
#else
    clock_gettime(CLOCK_MONOTONIC, &_t);
#endif

    return _t.tv_sec*1000 + lround(_t.tv_nsec/1.0e6);
}

uint64_t Linux_TimeUs()
{
    struct timespec _t;

#if defined(_MSC_VER) && !defined(WIN64_MINGW)
    timespec_get(&_t, TIME_UTC);
#else
    clock_gettime(CLOCK_MONOTONIC, &_t);
#endif

    return _t.tv_sec*1000000 + lround(_t.tv_nsec/1.0e3);
}

static stdFile_t Linux_stdFileOpen(const char* fpath, const char* mode)
{
    char tmp[512];


    size_t len = strlen(fpath);

    if (len > sizeof(tmp) - 1)
        len = sizeof(tmp) - 1;

    _strncpy(tmp, fpath, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    for (size_t i = 0; i < len; i++)
    {
        if (tmp[i] == '\\')
            tmp[i] = '/';
    }


    stdFile_t ret = 0;

    ret = (stdFile_t)fcaseopen(tmp, mode);

    return ret;
}

static int Linux_stdFileClose(stdFile_t fhand)
{
    int ret = fclose((FILE*)fhand);


    return ret;
}


static size_t Linux_stdFileRead(stdFile_t fhand, void* dst, size_t len)
{
    size_t val =  fread(dst, 1, len, (FILE*)fhand);

    return val;
}

static size_t Linux_stdFileWrite(stdFile_t fhand, void* dst, size_t len)
{
    return fwrite(dst, 1, len, (FILE*)fhand);
}

static const char* Linux_stdFileGets(stdFile_t fhand, char* dst, size_t len)
{
    // Drops static.jkl animclass parsing from 21.87s to 13.578s due to slow locks on getc
    return fgets(dst, len, (FILE*)fhand);
}

static const wchar_t* Linux_stdFileGetws(stdFile_t fhand, wchar_t* dst, size_t len)
{
    // Can't use fgetws because -fshort-wchar makes wchar_t 2 bytes
    // but libc fgetws expects native wchar_t (4 bytes on POSIX).
    // Read UTF-16LE characters one at a time instead.
    if (!len) return NULL;
    size_t i = 0;
    while (i < len - 1) {
        wchar_t ch = 0;
        if (fread(&ch, sizeof(wchar_t), 1, (FILE*)fhand) != 1) {
            if (i == 0) return NULL;
            break;
        }
        dst[i++] = ch;
        if (ch == L'\n') break;
    }
    dst[i] = L'\0';
    return dst;
}

static int Linux_stdFseek(stdFile_t fhand, int a, int b)
{
    //printf("fseek? %x %x\n", a, b);
    int ret = fseek((FILE*)fhand, a, b);
    //printf("fseek %x\n", ret);
    return ret;
}

static int Linux_stdFtell(stdFile_t fhand)
{
    return ftell((FILE*)fhand);
}

static void* Linux_alloc(uint32_t len)
{
    void* ret = malloc(len);
    if (ret) {
        memset(ret, 0, len);
    }
    return ret;
}

static void Linux_free(void* ptr)
{
    return free(ptr);
}

static void* Linux_realloc(void* ptr, uint32_t len)
{
    return realloc(ptr, len);
}



static int Linux_stdFeof(stdFile_t fhand)
{
    return feof((FILE*)fhand);
}

uint32_t stdPlatform_GetTimeMsec()
{
    return Linux_TimeMs();
}

#ifdef STDPLATFORM_HEAP_SUGGESTIONS
static int Dummy_suggestHeap(int which) { return HEAP_ANY; }
#endif


void stdPlatform_InitServices(HostServices *handlers)
{
    handlers->statusPrint = stdPlatform_Printf;
    handlers->messagePrint = stdPlatform_Printf;
    handlers->warningPrint = stdPlatform_Printf;
    handlers->errorPrint = stdPlatform_Printf;
    handlers->some_float = 1000.0;
    handlers->debugPrint = 0;
    handlers->unk_0 = 0;

    handlers->alloc = Linux_alloc;
    handlers->free = Linux_free;
    handlers->realloc = Linux_realloc;
    handlers->fileOpen = Linux_stdFileOpen;
    handlers->fileClose = Linux_stdFileClose;
    handlers->fileRead = Linux_stdFileRead;
    handlers->fileGets = Linux_stdFileGets;
    handlers->fileWrite = Linux_stdFileWrite;
    handlers->fileGetws = Linux_stdFileGetws;
    handlers->fseek = Linux_stdFseek;
    handlers->ftell = Linux_stdFtell;
    handlers->getTimerTick = Linux_TimeMs;
    handlers->fileEof = Linux_stdFeof;

#ifdef STDPLATFORM_HEAP_SUGGESTIONS
   handlers->suggestHeap = Dummy_suggestHeap;
#endif


}

int stdPlatform_Startup()
{
    return 1;
}

int stdPrintf(int (*a1)(const char *, ...), const char *a2, int line, const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    printf("(%p %s:%d) ", a1, a2, line);
    int ret = vprintf(fmt, args);
    va_end (args);
    return ret;
}

static SDL_mutex* stdPlatform_mtxPrintf = NULL;

int stdPlatform_Printf(const char *fmt, ...)
{
    char tmp[256];
    va_list args;

    if (!stdPlatform_mtxPrintf)
        stdPlatform_mtxPrintf = SDL_CreateMutex();

    SDL_LockMutex(stdPlatform_mtxPrintf);
    
    va_start (args, fmt);
    int ret = vprintf(fmt, args);
    va_end (args);

#ifdef QUAKE_CONSOLE
    va_start (args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    jkQuakeConsole_PrintLine(tmp);
    va_end(args);
#endif



    SDL_UnlockMutex(stdPlatform_mtxPrintf);
    return ret;
}




// Added: per-file allocation cataloguing. The original game clearly had debug
// allocation hooks in its host-services design; this reconstructs the idea.
// Enabled by STDPLATFORM_ALLOC_TRACKING (the *_ALLOC/*_FREE macros in
// stdPlatform.h); stats keyed on __FILE__ string identity (each TU passes its
// own literal, so pointer compare hits first; strcmp is the cross-TU fallback).
// Sizes at free/realloc come from stdPlatform_AllocSize below, which reads the
// platform allocator's own header -- no extra per-allocation overhead that
// would distort the memory pressure being measured.

// Read back the user-byte size of an allocation from the underlying allocator.
// Returns 0 if unknown (untracked/foreign pointer) -- callers treat 0 as "skip".
uint32_t stdPlatform_AllocSize(void* p)
{
    if (!p) return 0;
#if defined(__APPLE__)
    extern size_t malloc_size(const void*);
    return (uint32_t)malloc_size(p);
#elif defined(__GLIBC__)
    extern size_t malloc_usable_size(void*);
    return (uint32_t)malloc_usable_size(p);
#elif defined(_MSC_VER)
    return (uint32_t)_msize(p);
#else
    return 0;
#endif
}

#define STDALLOCTRACK_MAX_FILES 128
typedef struct stdAllocTrackEnt
{
    const char* pFile;
    int32_t liveBytes;
    int32_t liveCount;
    int32_t peakBytes;
    uint32_t totalAllocs;
} stdAllocTrackEnt;
static stdAllocTrackEnt stdPlatform_aAllocTrack[STDALLOCTRACK_MAX_FILES];
static int stdPlatform_numAllocTrack = 0;
static int32_t stdPlatform_allocTrackUntracked = 0; // freed-without-size etc.

static stdAllocTrackEnt* stdPlatform_AllocTrackEnt(const char* pFile)
{
    for (int i = 0; i < stdPlatform_numAllocTrack; i++) {
        if (stdPlatform_aAllocTrack[i].pFile == pFile)
            return &stdPlatform_aAllocTrack[i];
    }
    for (int i = 0; i < stdPlatform_numAllocTrack; i++) {
        if (!_strcmp(stdPlatform_aAllocTrack[i].pFile, pFile))
            return &stdPlatform_aAllocTrack[i];
    }
    if (stdPlatform_numAllocTrack >= STDALLOCTRACK_MAX_FILES)
        return NULL;
    stdAllocTrackEnt* ent = &stdPlatform_aAllocTrack[stdPlatform_numAllocTrack++];
    ent->pFile = pFile;
    return ent;
}

void* stdPlatform_TrackedAlloc(void* (*allocFn)(uint32_t), uint32_t len, const char* pFile)
{
    void* p = allocFn(len);
    if (p) {
        stdAllocTrackEnt* ent = stdPlatform_AllocTrackEnt(pFile);
        if (ent) {
            ent->liveBytes += (int32_t)len;
            ent->liveCount++;
            ent->totalAllocs++;
            if (ent->liveBytes > ent->peakBytes)
                ent->peakBytes = ent->liveBytes;
        }
    }
    return p;
}

void stdPlatform_TrackedFree(void (*freeFn)(void*), void* p, const char* pFile)
{
    if (p) {
        uint32_t sz = stdPlatform_AllocSize(p);
        stdAllocTrackEnt* ent = stdPlatform_AllocTrackEnt(pFile);
        if (ent && sz) {
            ent->liveBytes -= (int32_t)sz;
            ent->liveCount--;
        }
        else if (!sz) {
            stdPlatform_allocTrackUntracked++;
        }
    }
    freeFn(p);
}

void* stdPlatform_TrackedRealloc(void* (*reallocFn)(void*, uint32_t), void* p, uint32_t len, const char* pFile)
{
    uint32_t oldSz = p ? stdPlatform_AllocSize(p) : 0;
    void* pNew = reallocFn(p, len);
    if (pNew) {
        stdAllocTrackEnt* ent = stdPlatform_AllocTrackEnt(pFile);
        if (ent) {
            ent->liveBytes += (int32_t)len - (int32_t)oldSz;
            if (!p) ent->liveCount++;
            ent->totalAllocs++;
            if (ent->liveBytes > ent->peakBytes)
                ent->peakBytes = ent->liveBytes;
        }
    }
    return pNew;
}

// Dump the catalog, largest live footprint first.
void stdPlatform_PrintAllocStats(void)
{
    int aOrder[STDALLOCTRACK_MAX_FILES];
    int32_t totalLive = 0, totalPeak = 0;
    for (int i = 0; i < stdPlatform_numAllocTrack; i++)
        aOrder[i] = i;
    for (int i = 1; i < stdPlatform_numAllocTrack; i++) {
        int v = aOrder[i], j = i - 1;
        while (j >= 0 && stdPlatform_aAllocTrack[aOrder[j]].liveBytes < stdPlatform_aAllocTrack[v].liveBytes) {
            aOrder[j + 1] = aOrder[j];
            j--;
        }
        aOrder[j + 1] = v;
    }
    stdPlatform_Printf("=== alloc catalog (live KB / peak KB / count / total allocs) ===\n");
    for (int i = 0; i < stdPlatform_numAllocTrack; i++) {
        stdAllocTrackEnt* ent = &stdPlatform_aAllocTrack[aOrder[i]];
        const char* pName = ent->pFile;
        for (const char* c = pName; *c; c++)
            if (*c == '/' || *c == '\\') pName = c + 1;
        stdPlatform_Printf("%7.1f %7.1f %5d %6u  %s\n",
            ent->liveBytes / 1024.0, ent->peakBytes / 1024.0,
            ent->liveCount, ent->totalAllocs, pName);
        totalLive += ent->liveBytes;
        totalPeak += ent->peakBytes;
    }
    stdPlatform_Printf("=== total live %d KB (sum of peaks %d KB, %d untracked frees) ===\n",
        (int)(totalLive / 1024), (int)(totalPeak / 1024), stdPlatform_allocTrackUntracked);
}

// Added: does this pointer live in memory that drops byte-granular stores?
// Callers use it to decide between a direct fileRead and a word-safe bounce.
int stdPlatform_IsWordAddressableOnly(const void* p)
{
    (void)p;
    return 0;
}

// Added:
// memcpy/memset that never issue byte stores, for word-addressable-only
// destinations (Dreamcast VRAM, NDS slot-2 RAM -- their buses drop byte enables
// on writes; byte reads are fine). Exact length, any src/dst alignment: unaligned
// edges use 16-bit read-modify-write, bulk uses 32-bit stores (little-endian).
void stdPlatform_Memzero32(void* dst, uint32_t len)
{
    uint8_t* pDst = (uint8_t*)dst;
    if (len && ((uintptr_t)pDst & 1)) {
        stdPlatform_WriteByte16(pDst, 0);
        pDst++; len--;
    }
    if (len >= 2 && ((uintptr_t)pDst & 2)) {
        *(uint16_t*)pDst = 0;
        pDst += 2; len -= 2;
    }
    for (; len >= 4; len -= 4, pDst += 4)
        *(uint32_t*)pDst = 0;
    if (len >= 2) {
        *(uint16_t*)pDst = 0;
        pDst += 2; len -= 2;
    }
    if (len)
        stdPlatform_WriteByte16(pDst, 0);
}

void stdPlatform_Memset32(void* dst, uint8_t val, uint32_t len)
{
    uint8_t* pDst = (uint8_t*)dst;
    uint32_t pattern = val * 0x01010101u;
    if (len && ((uintptr_t)pDst & 1)) {
        stdPlatform_WriteByte16(pDst, val);
        pDst++; len--;
    }
    if (len >= 2 && ((uintptr_t)pDst & 2)) {
        *(uint16_t*)pDst = (uint16_t)pattern;
        pDst += 2; len -= 2;
    }
    for (; len >= 4; len -= 4, pDst += 4)
        *(uint32_t*)pDst = pattern;
    if (len >= 2) {
        *(uint16_t*)pDst = (uint16_t)pattern;
        pDst += 2; len -= 2;
    }
    if (len)
        stdPlatform_WriteByte16(pDst, val);
}

void stdPlatform_Memcpy32(void* dst, const void* src, uint32_t len)
{
    uint8_t* pDst = (uint8_t*)dst;
    const uint8_t* pSrc = (const uint8_t*)src;
    if (len && ((uintptr_t)pDst & 1)) {
        stdPlatform_WriteByte16(pDst, *pSrc);
        pDst++; pSrc++; len--;
    }
    if (len >= 2 && ((uintptr_t)pDst & 2)) {
        *(uint16_t*)pDst = (uint16_t)pSrc[0] | ((uint16_t)pSrc[1] << 8);
        pDst += 2; pSrc += 2; len -= 2;
    }
    if (!((uintptr_t)pSrc & 3)) {
        // Fast path: src co-aligned, straight 32-bit copies
        for (; len >= 4; len -= 4, pDst += 4, pSrc += 4)
            *(uint32_t*)pDst = *(const uint32_t*)pSrc;
    } else {
        // Src misaligned: assemble words from byte reads (always safe)
        for (; len >= 4; len -= 4, pDst += 4, pSrc += 4)
            *(uint32_t*)pDst = (uint32_t)pSrc[0] | ((uint32_t)pSrc[1] << 8) |
                               ((uint32_t)pSrc[2] << 16) | ((uint32_t)pSrc[3] << 24);
    }
    if (len >= 2) {
        *(uint16_t*)pDst = (uint16_t)pSrc[0] | ((uint16_t)pSrc[1] << 8);
        pDst += 2; pSrc += 2; len -= 2;
    }
    if (len)
        stdPlatform_WriteByte16(pDst, *pSrc);
}
#include "stdGob.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jk.h"
#include "stdPlatform.h"

#include "General/stdHashTable.h"
#include "General/stdString.h"
#include "Platform/Common/stdEmbeddedRes.h"

static HostServices gobHS;
static HostServices* pGobHS;
static int stdGob_bInit;
static char stdGob_fpath[128];


int stdGob_Startup(HostServices *pHS_in)
{
    _memcpy(&gobHS, pHS_in, sizeof(gobHS));
    pGobHS = &gobHS;
    stdGob_bInit = 1;
    return 1;
}

void stdGob_Shutdown()
{
    stdGob_bInit = 0;
}

stdGob* stdGob_Load(char *fpath, int a2, int a3)
{
    stdGob* gob = (stdGob*)STD_ALLOC(sizeof(stdGob));
    if (gob)
    {
        _memset(gob, 0, sizeof(stdGob)); // TODO why was this needed
        stdGob_LoadEntry(gob, fpath, a2, a3); // TODO verify this? it does weird stuff
        return gob;
    }
    return NULL;
}

int stdGob_LoadEntry(stdGob *gob, char *fname, int a3, int a4)
{
    int v8; // edx
    stdGobFile *v9; // eax
    stdGobHeader header; // [esp+10h] [ebp-Ch]

    stdString_SafeStrCopy(gob->fpath, fname, 128);
    gob->numFilesOpen = a3;
    gob->lastReadFile = 0;

    //TODO fix this? WINE/df2_reimpl.dll keeps corrupting the gobs? Might be something else idk.

    gob->viewMapped = 0;
    gob->fhand = pGobHS->fileOpen(gob->fpath, "rb"); // Added: r+b -> rb, we don't actually need to write GOBs
    if ( !gob->fhand ) {
        stdPlatform_Printf("OpenJKDF2: stdGob failed to open `%s`.\n", gob->fpath); // Added
        return 0;
    }
    else {
        stdPlatform_Printf("OpenJKDF2: stdGob opened `%s`.\n", gob->fpath); // Added
    }
    gob->openedFile = (stdGobFile *)STD_ALLOC(sizeof(stdGobFile) * gob->numFilesOpen);
    if ( !gob->openedFile )
      return 0;
    _memset(gob->openedFile, 0, sizeof(stdGobFile) * gob->numFilesOpen);
    pGobHS->fileRead(gob->fhand, &header, sizeof(stdGobHeader));
    if ( _memcmp((const char *)&header, "GOB ", 4u) )
    {
      stdPrintf(std_pHS->errorPrint, ".\\Win95\\stdGob.c", 270, "Error: Bad signature in header of gob file.\n", 0, 0, 0, 0);
      return 0;
    }
    if ( header.version != 20 )
    {
      stdPrintf(std_pHS->errorPrint, ".\\Win95\\stdGob.c", 277, "Error: Bad version %d for gob file\n", header.version, 0, 0, 0);
      return 0;
    }
    pGobHS->fseek(gob->fhand, header.entryTable_offs, 0);
    pGobHS->fileRead(gob->fhand, &gob->numFiles, sizeof(uint32_t));
    gob->entries = (stdGobEntry *)STD_ALLOC(sizeof(stdGobEntry) * gob->numFiles);
    if ( !gob->entries )
      return 0;
    
    // Added
    _memset(gob->entries, 0, sizeof(stdGobEntry) * gob->numFiles);

    // We're not adding anything so like, keep it small?
    gob->entriesHashtable = stdHashTable_New(1024);
    for (int v4 = 0; v4 < gob->numFiles; v4++)
    {
#ifdef STDGOB_COMPACT_ENTRIES
        // Added: stage the fixed 136-byte disk entry; only offset/size stay
        // resident (the CRC-keyed hashtable doesn't retain the name pointer).
        stdGobDiskEntry diskEntry;
        pGobHS->fileRead(gob->fhand, &diskEntry, sizeof(stdGobDiskEntry));
        gob->entries[v4].fileOffset = diskEntry.fileOffset;
        gob->entries[v4].fileSize = diskEntry.fileSize;
        stdHashTable_SetKeyVal(gob->entriesHashtable, diskEntry.fname, &gob->entries[v4]);
#else
        pGobHS->fileRead(gob->fhand, &gob->entries[v4], sizeof(stdGobEntry));
        stdHashTable_SetKeyVal(gob->entriesHashtable, gob->entries[v4].fname, &gob->entries[v4]);
#endif
    }

    stdPlatform_Printf("OpenJKDF2: stdGob loaded GOB file `%s`...\n", fname);
    
    return 1;
}

void stdGob_Free(stdGob *gob)
{
    if (!gob )
        return;

    stdGob_FreeEntry(gob);
    STD_FREE(gob);
}

void stdGob_FreeEntry(stdGob *gob)
{
    if ( gob->viewMapped )
    {
        jk_UnmapViewOfFile(gob->viewAddr);
        jk_CloseHandle(gob->viewHandle);
        jk_CloseHandle(gob->viewHandle2);
    }
    else
    {
        // Added: Fix file handle leak
        if (gob->fhand) {
            pGobHS->fileClose(gob->fhand);
            gob->fhand = 0;
        }
        // Added: Fix memleak
        if (gob->openedFile) {
            STD_FREE(gob->openedFile);
            gob->openedFile = NULL;
        }
        if ( gob->entries )
        {
            STD_FREE(gob->entries);
            gob->entries = 0;
        }
        if ( gob->entriesHashtable )
        {
            stdHashTable_Free(gob->entriesHashtable);
            gob->entriesHashtable = 0;
        }
    }
}

stdGobFile* stdGob_FileOpen(stdGob *gob, const char *filepath)
{
    stdGobEntry *entry = NULL;
    stdGobFile *result = NULL;
    int v5;

    // Embedded resources
    size_t sz = 0;
    void* data = stdEmbeddedRes_LoadOnlyInternal(filepath, &sz);
    if (data) {
        result = gob->openedFile;
        v5 = 0;
        if ( !gob->numFilesOpen )
            return 0;

        while ( result->isOpen )
        {
            ++result;
            if ( ++v5 >= gob->numFilesOpen )
                return 0;
        }
        result->bIsMemoryMapped = 1;
        result->pMemory = (intptr_t)data;
        result->memorySz = sz;

        result->isOpen = 1;
        result->parent = gob;
        result->entry = entry;
        result->seekOffs = 0;
        // Added: Opening another file in this GOB makes the shared handle's position
        // ambiguous: invalidate the seek-skip cache so the next read re-seeks.
        gob->lastReadFile = 0;
        return result;
    }

    // Added: clean up paths
    if (filepath[0] == '.' && (filepath[1] == '/' || filepath[1] == '\\')) {
        filepath += 2;
    }
    stdString_SafeStrCopy(stdGob_fpath, filepath, 128);
    stdString_CStrToLower(stdGob_fpath);

    for (int i = 0; i < 128; i++)
    {
        if (stdGob_fpath[i] == '/')
            stdGob_fpath[i] = '\\';
    }
    entry = (stdGobEntry*)stdHashTable_GetKeyVal(gob->entriesHashtable, stdGob_fpath);
    if (!entry)
        return 0;

    result = gob->openedFile;
    v5 = 0;
    if ( !gob->numFilesOpen )
        return 0;

    while ( result->isOpen )
    {
        ++result;
        if ( ++v5 >= gob->numFilesOpen )
            return 0;
    }
    result->bIsMemoryMapped = 0;
    result->pMemory = (intptr_t)NULL;
    result->memorySz = 0;
    result->isOpen = 1;
    result->parent = gob;
    result->entry = entry;
    result->seekOffs = 0;
    // Added: Opening another file in this GOB makes the shared handle's position
    // ambiguous: invalidate the seek-skip cache so the next read re-seeks.
    gob->lastReadFile = 0;
    return result;
}

void stdGob_FileClose(stdGobFile *f)
{
    if (f->pMemory) {
        free((void*)f->pMemory);
        f->pMemory = (intptr_t)NULL;
    }
    f->bIsMemoryMapped = 0;

    stdGob* gob = f->parent;
    f->isOpen = 0;

    if (f == gob->lastReadFile) {
        gob->lastReadFile = 0;
    }
}

int stdGob_FSeek(stdGobFile *f, int pos, int whence)
{
    int seekOffsAbsolute;
    stdGob *gob;

    seekOffsAbsolute = 0;
    switch (whence)
    {
        case SEEK_SET:
            seekOffsAbsolute = pos;
            break;
        case SEEK_CUR:
            seekOffsAbsolute = pos + f->seekOffs;
            break;
        case SEEK_END:
            seekOffsAbsolute = pos + f->entry->fileSize;
            break;
        default:
            return 0;
    }

    gob = f->parent;
    f->seekOffs = seekOffsAbsolute;

    if (f == gob->lastReadFile)
        gob->lastReadFile = 0;

    return 1;
}

int32_t stdGob_FTell(stdGobFile *f)
{
    return f->seekOffs;
}

bool stdGob_FEof(stdGobFile *f)
{
    int ret = 0;
    ret = f->seekOffs >= f->entry->fileSize - 1;
    return ret;
}

size_t stdGob_FileRead(stdGobFile *f, void *out, uint32_t len)
{
    stdGob *gob;
    size_t result;

    if (f->bIsMemoryMapped) {
        size_t to_read = len;
        if (f->seekOffs >= f->memorySz) {
            f->seekOffs = f->memorySz;
            return 0;
        }

        if (f->seekOffs + to_read > f->memorySz) {
            to_read = f->memorySz - f->seekOffs;
        }
        memcpy(out, (void*)(f->pMemory + f->seekOffs), to_read);
        f->seekOffs += to_read;

        return to_read;
    }

    gob = f->parent;
    if (gob->lastReadFile != f)
    {
        pGobHS->fseek(gob->fhand, f->seekOffs + f->entry->fileOffset, SEEK_SET);
        gob = f->parent;
        gob->lastReadFile = f;
    }

    if (f->entry->fileSize - f->seekOffs < len)
        len = f->entry->fileSize - f->seekOffs;

    result = pGobHS->fileRead(gob->fhand, out, len);
    f->seekOffs += result;
    return result;
}

const char* stdGob_FileGets(stdGobFile *f, char *out, unsigned int len)
{
    stdGobEntry *entry;
    int seekOffs;
    const char *result;
    stdGob *gob;

    if (f->bIsMemoryMapped) {
        size_t remaining;
        size_t max_read;
        size_t actual_read;
        const char* src;
        const char* newline;

        if (!out || len == 0)
            return NULL;

        if (f->seekOffs < 0 || (size_t)f->seekOffs >= f->memorySz) {
            f->seekOffs = (int32_t)f->memorySz;
            return NULL;
        }

        /*
         * fgets() may store at most len - 1 data bytes,
         * reserving one byte for the terminating NUL.
         */
        remaining = f->memorySz - (size_t)f->seekOffs;
        max_read = len - 1;

        if (max_read > remaining)
            max_read = remaining;

        if (max_read == 0) {
            out[0] = '\0';
            return NULL;
        }

        src = (const char*)(f->pMemory + f->seekOffs);

        /*
         * Search only inside the valid mapped range.
         * Include '\n' in the returned line, like fgets().
         */
        newline = (const char*)memchr(src, '\n', max_read);

        if (newline)
            actual_read = (size_t)(newline - src) + 1;
        else
            actual_read = max_read;

        memcpy(out, src, actual_read);
        out[actual_read] = '\0';

        f->seekOffs += (int32_t)actual_read;

        return out;
    }


    entry = f->entry;
    seekOffs = f->seekOffs;
    if (seekOffs >= entry->fileSize - 1)
        return 0;

    gob = f->parent;
    if (gob->lastReadFile != f)
    {
        pGobHS->fseek(gob->fhand, seekOffs + entry->fileOffset, SEEK_SET);
        gob = f->parent;
        gob->lastReadFile = f;
    }

    if (f->entry->fileSize - f->seekOffs + 1 < len)
        len = f->entry->fileSize - f->seekOffs + 1;

    result = pGobHS->fileGets(gob->fhand, out, len);
    if (result)
        f->seekOffs += _strlen(result);

    return result;
}

const wchar_t* stdGob_FileGetws(stdGobFile *f, wchar_t *out, unsigned int len)
{
    stdGobEntry *entry; // ecx
    int seekOffs; // edx
    stdGob *gob; // eax
    unsigned int seekOffs_; // edi
    unsigned int len_wide; // ecx
    const wchar_t *ret; // eax
    const wchar_t *ret_; // edi

    if (f->bIsMemoryMapped) {
        size_t to_read = len * sizeof(wchar_t);
        if (f->seekOffs >= f->memorySz) {
            f->seekOffs = f->memorySz;
            return 0;
        }

        if (f->seekOffs + to_read > f->memorySz) {
            to_read = f->memorySz - f->seekOffs;
        }
        if (!to_read) {
            return NULL;
        }
        __wcsncpy(out, (wchar_t*)(f->pMemory + f->seekOffs), to_read / sizeof(wchar_t));
        wchar_t* cutoff = __wcschr(out, '\n');
        if (cutoff) {
            *(++cutoff) = 0;
        }

        size_t actual_read = (_wcslen(out))*sizeof(wchar_t);
        f->seekOffs += actual_read;

        if (!actual_read) return NULL;

        return out;
    }

    entry = f->entry;
    seekOffs = f->seekOffs;
    if ( seekOffs >= entry->fileSize - 1 )
        return 0;
    gob = f->parent;
    if ( gob->lastReadFile != f )
    {
        pGobHS->fseek(gob->fhand, seekOffs + entry->fileOffset, 0);
        gob = f->parent;
        gob->lastReadFile = f;
    }
    seekOffs_ = f->seekOffs;
    len_wide = len;
    if ( ((f->entry->fileSize - seekOffs_) >> 1) + 1 < len )
        len_wide = ((f->entry->fileSize - seekOffs_) >> 1) + 1;
    ret = pGobHS->fileGetws(gob->fhand, out, len_wide);
    if (ret)
        f->seekOffs += _wcslen(ret);
    return ret;
}

// ADDED
size_t stdGob_FileSize(stdGobFile *f)
{
    if (!f) return 0;
    if (!f->entry) return 0;
    if (f->bIsMemoryMapped) {
        return f->memorySz;
    }

    return f->entry->fileSize;
}

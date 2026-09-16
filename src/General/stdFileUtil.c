#include "stdFileUtil.h"

#include "stdPlatform.h"
#include "General/stdFnames.h"
#include "General/stdString.h"
#include "jk.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ftw.h>

#include "external/fcaseopen/fcaseopen.h"


stdFileSearch* stdFileUtil_NewFind(const char *path, int a2, const char *extension)
{
    stdFileSearch* search = (stdFileSearch *)STD_ALLOC(sizeof(stdFileSearch));
    if ( !search ) {
        return search;
    }
    _memset(search, 0, sizeof(stdFileSearch));

    if ( a2 < 0 )
        return search;
    if ( a2 <= 2 )
    {
        stdFnames_MakePath(search->path, 128, path, "*.*");
        return search;
    }
    if ( a2 != 3 )
        return search;
    if ( *extension == '.' )
        extension = extension + 1;
    stdString_snprintf(std_genBuffer, 1024, "*.%s", extension);
    stdFnames_MakePath(search->path, 128, path, std_genBuffer);
    
#ifdef FS_POSIX
    for (int i = 0; i < strlen(search->path); i++)
    {
        if (search->path[i] == '\\')
            search->path[i] = '/';
    }
#endif



    stdPlatform_Printf("OpenJKDF2: %s %s\n", __func__, search->path);
    
    return search;
}




// Stolen from https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
static int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    if(remove(pathname) < 0)
    {
        perror("ERROR: remove");
        return -1;
    }
    return 0;
}

int stdFileUtil_Deltree(const char* lpPathName)
{
    char tmp[512];
    size_t len = _strlen(lpPathName);

    if (len > 512) {
        len = 512;
    }
    stdString_SafeStrCopy(tmp, lpPathName, sizeof(tmp));

    for (int i = 0; i < len; i++)
    {
        if (tmp[i] == '\\') {
            tmp[i] = '/';
        }
    }

    nftw(tmp, rmFiles, 10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS);

    //rmdir(tmp);
    return 0;
}


static char* search_ext = "";

/* when return 1, scandir will put this dirent to the list */
static int parse_ext(const struct dirent *dir)
{
    if(!dir)
        return 0;

    if(dir->d_type == DT_REG) 
    {
        const char *ext = strrchr(dir->d_name,'.');
        if((!ext) || (ext == dir->d_name)) {
            return 0;
        }
        else 
        {
            if(__strnicmp(ext, search_ext, 3) == 0)
                return 1;
        }
    }
    else
    {
        if (!strncmp(dir->d_name, ".", 1)) return 1;
        if (!strncmp(dir->d_name, "..", 1)) return 1;
    }

    return 0;
}

int stdFileUtil_FindNext(stdFileSearch *a1, stdFileSearchResult *a2)
{
    struct dirent *iter;
    char tmp[128];

    if ( !a1 )
        return 0;

    if (a1->isNotFirst++)
    {
        if (a1->isNotFirst >= a1->num_found)
            iter = NULL;
        else
            iter = a1->namelist[a1->isNotFirst];
    }
    else
    {
        strncpy(tmp, a1->path, 128);

        // Clear out extension
        // TODO: ehhhh
        if (!strcmp(strrchr(tmp,'*'), "*")) {
            *strrchr(tmp,'.') = 0;
            *strrchr(tmp,'*') = 0;
            search_ext = strrchr(a1->path,'.');
            search_ext = NULL;
        }
        else
        {
            *strrchr(tmp,'.') = 0;
            *strrchr(tmp,'*') = 0;
            search_ext = strrchr(a1->path,'.');
        }
        
        for (int i = 0; i < strlen(tmp); i++)
        {
            if (tmp[i] == '\\') {
                tmp[i] = '/';
            }
        }
        if (tmp[strlen(tmp)-1] = '/') {
            tmp[strlen(tmp)-1] = 0;
        }

        a1->num_found = scandir(tmp, &a1->namelist, search_ext ? parse_ext : NULL, alphasort);
        
        if (!a1->namelist || a1->num_found <= 0) return 0;
        
        iter = a1->namelist[2];
        a1->isNotFirst = 2;
    }

    if (a1->num_found <= 2 || !iter)
        return 0;

    strncpy(a2->fpath, iter->d_name, sizeof(a2->fpath));

    a2->time_write = 0;
    a2->is_subdirectory = iter->d_type == DT_DIR ? 0x10 : 0;

    return 1;
}

void stdFileUtil_DisposeFind(stdFileSearch *search)
{
    if ( search )
    {
        for (int i = 0; i < search->num_found; i++)
        {
           free(search->namelist[i]);
        }
        free(search->namelist);

        STD_FREE(search);
    }
}

void stdFileUtil_FindReset(stdFileSearch *search)
{
    if ( search )
    {
        for (int i = 0; i < search->num_found; i++)
        {
            free(search->namelist[i]);
        }
        free(search->namelist);
        search->namelist = NULL;
        search->isNotFirst = 0;
        search->num_found = 0;
    }
}

int stdFileUtil_FindQuick(const char *path, int type, const char *extension, stdFileSearchResult *result)
{
    stdFileSearch *search = stdFileUtil_NewFind(path, type, extension);
    if ( !search )
        return 0;

    int found = stdFileUtil_FindNext(search, result);
    stdFileUtil_DisposeFind(search);
    return found;
}

int stdFileUtil_CountMatches(const char *path, int type, const char *extension)
{
    stdFileSearchResult result;
    stdFileSearch *search = stdFileUtil_NewFind(path, type, extension);
    if ( !search )
        return 0;

    int count = 0;
    while ( stdFileUtil_FindNext(search, &result) )
    {
        count++;
    }
    stdFileUtil_DisposeFind(search);
    return count;
}

int stdFileUtil_DirExists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

void stdFileUtil_RmDir(const char *path)
{
    rmdir(path);
}

// https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
static void _mkdir(const char *dir, int perms) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, perms);
            *p = '/';
        }
    mkdir(tmp, perms);
}

int stdFileUtil_MkDir(char* path)
{
    char tmp[512];
    size_t len = _strlen(path);

    if (len > 512) {
        len = 512;
    }
    _strncpy(tmp, path, sizeof(tmp));

    for (int i = 0; i < len; i++)
    {
        if (tmp[i] == '\\') {
            tmp[i] = '/';
        }
    }

    _mkdir(tmp, 0777);

    return 1;
}

int stdFileUtil_DelFile(char* lpFileName)
{
    char tmp[512];
    size_t len = _strlen(lpFileName);

    if (len > 512) {
        len = 512;
    }
    _strncpy(tmp, lpFileName, sizeof(tmp));

    for (int i = 0; i < len; i++)
    {
        if (tmp[i] == '\\') {
            tmp[i] = '/';
        }
    }

    unlink(tmp);

    return 1;
}

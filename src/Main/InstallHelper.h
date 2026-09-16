#ifndef _MAIN_INSTALLHELPER_H

#include "types.h"

#include <locale.h>

#include "SDL2_helper.h"
#include <sys/types.h>
#include <stdbool.h>
#include <pwd.h>
#include "nfd.h"

#include "external/fcaseopen/fcaseopen.h"


int InstallHelper_copy(const char* in_path, const char* out_path);
int InstallHelper_CopyFile(const char* pFolder, const char* pName);
int InstallHelper_CopyFileDisk(const char* pFolder, const char* pName);
int InstallHelper_GetLocalDataDir(char* pOut, size_t pOut_sz, int bChdir);
int InstallHelper_UseLocalData();
int InstallHelper_AttemptInstallFromExisting(nfdu8char_t* path);
int InstallHelper_AttemptInstallFromDisk(nfdu8char_t* path);
int InstallHelper_AttemptInstall();
void InstallHelper_CheckRequiredAssets(int doInstall);
void InstallHelper_SetCwd();

#endif // _MAIN_INSTALLHELPER_H
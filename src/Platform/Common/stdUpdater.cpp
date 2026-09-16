#include "stdUpdater.h"

extern "C" {

void stdUpdater_StartupCvars()
{
}

int stdUpdater_CheckForUpdates()
{
    return 0;
}

void stdUpdater_GetUpdateText(wchar_t* pOut, size_t outSz)
{
    if (pOut && outSz >= sizeof(wchar_t))
        pOut[0] = 0;
}

void stdUpdater_DoUpdate()
{
}

}

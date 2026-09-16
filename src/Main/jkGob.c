#include "jkGob.h"

#include "Main/jkGame.h"
#include "Win95/stdGob.h"

static int jkGob_bInit;

int jkGob_Startup()
{
    stdPlatform_Printf("OpenJKDF2: %s\n", __func__);
    
    stdGob_Startup(pHS);
    jkGob_bInit = 1;
    return 1;
}

void jkGob_Shutdown()
{
    stdPlatform_Printf("OpenJKDF2: %s\n", __func__);

}

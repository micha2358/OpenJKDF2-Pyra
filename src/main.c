#include <stdint.h>
#include <stdio.h>

#include "hook.h"
#include "jk.h"
#include "types.h"

#include "General/stdMath.h"

#include "General/stdString.h"
#include "Win95/Window.h"
#include "Main/jkMain.h"
#include "Main/Main.h"

extern char openjkdf2_aOrigCwd[512];

void do_hooks();



#ifdef PLATFORM_PHYSFS
#include <physfs.h>
#endif

void crash_handler_basic(int sig);

#include <sys/mman.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "SDL2_helper.h"


// HACK: minGW fails on the Github runner??



int main(int argc, char** argv)
{
    /* The startup working directory is the Pyra data directory. */
    char pyraDataDir[sizeof(openjkdf2_aOrigCwd)] = {0};
    if (!getcwd(pyraDataDir, sizeof(pyraDataDir))) {
        fprintf(stderr, "OpenJKDF2: ERROR: could not determine startup/data directory: %s\n",
                strerror(errno));
        return 1;
    }
    printf("OpenJKDF2: Pyra data directory: %s\n", pyraDataDir);

#if defined(TARGET_TWL) && defined(TWL_AUTOBOOT_MAP)
    // Added: NDS/DSi twin of DC_AUTOBOOT_MAP (TWL_AUTOBOOT_MAP env at build time)
    static char* twlAutobootArgv[] = { "openjkdf2", "-autostart", "-sp",
                                       "-episode", "JK1", "-map", TWL_AUTOBOOT_MAP };
    argc = 7;
    argv = twlAutobootArgv;
    extern int jkGuiTitle_bSkipLoadingWait; // Added: don't wait for input at the load screen
    jkGuiTitle_bSkipLoadingWait = 1;
#endif





    signal(SIGSEGV, crash_handler_basic);
    //signal(SIGINT, int_handler);

#ifndef ARCH_64BIT
#endif // !ARCH_64BIT

#ifdef PLATFORM_PHYSFS
    PHYSFS_init(argv[0]);
    PHYSFS_permitSymbolicLinks(0);
#endif

    /* Restore and retain the original startup directory as canonical data dir. */
    if (chdir(pyraDataDir) != 0) {
        fprintf(stderr, "OpenJKDF2: ERROR: could not restore Pyra data directory `%s`: %s\n",
                pyraDataDir, strerror(errno));
        return 1;
    }
    stdString_SafeStrCopy(openjkdf2_aOrigCwd, pyraDataDir,
                          sizeof(openjkdf2_aOrigCwd));
    printf("OpenJKDF2: using current working directory as data directory: %s\n",
           openjkdf2_aOrigCwd);

    openjkdf2_bOrigWasDF2 = 1;
    for (int i = 1; i < argc; i++) {
        if (!__strcmpi(argv[i], "-motsCompat") || !__strcmpi(argv[i], "/motsCompat")) {
            openjkdf2_bOrigWasDF2 = 0; // Running MoTS.
        }
        else if (!__strcmpi(argv[i], "-path") || !__strcmpi(argv[i], "/path")) {
            openjkdf2_bOrigWasDF2 = 0; // Running some kind of mod.
        }
    }

    while (1)
    {
        OpenJKDF2_Globals_Reset();

        // Set the mod path
        if (openjkdf2_restartMode == OPENJKDF2_RESTART_PATH) {
            stdString_SafeStrCopy(Main_path, openjkdf2_aRestartPath, 128);
        }

        openjkdf2_restartMode = OPENJKDF2_RESTART_NONE;
        memset(openjkdf2_aRestartPath, 0, sizeof(openjkdf2_aRestartPath));
        /* Relative Resource/Episode/MUSIC/mods paths must resolve from data dir. */
        if (chdir(openjkdf2_aOrigCwd) != 0) {
            fprintf(stderr, "OpenJKDF2: ERROR: could not enter Pyra data directory `%s`: %s\n",
                    openjkdf2_aOrigCwd, strerror(errno));
            return 1;
        }
        Window_Main_Linux(argc, argv);

        printf("openjkdf2_bOrigWasRunningFromExistingInstall %x\n", openjkdf2_bOrigWasRunningFromExistingInstall);
        printf("openjkdf2_bIsRunningFromExistingInstall %x\n", openjkdf2_bIsRunningFromExistingInstall);
        printf("openjkdf2_bOrigWasDF2 %x\n", openjkdf2_bOrigWasDF2);

        openjkdf2_bIsFirstLaunch = 0;
        if (openjkdf2_restartMode != OPENJKDF2_RESTART_NONE) {
            // Purge any cmdline args that will get in the way.
            for (int i = 1; i < argc; i++) {
                if (!__strcmpi(argv[i], "-motsCompat") || !__strcmpi(argv[i], "/motsCompat")) {
                    argv[i] = "";
                }
                else if (!__strcmpi(argv[i], "-path") || !__strcmpi(argv[i], "/path")) {
                    argv[i] = "";
                    argv[i+1] = "";
                }
            }

            // Scenario: User has an existing JK.EXE/JKM.EXE replacement install, and wants to run the other game.
            // If we keep the current working directory, it will trigger the jk_.cd checks and not restart correctly.
            // So we override the cwd to the LocalData directory to prevent this.
            if ((openjkdf2_bOrigWasRunningFromExistingInstall && openjkdf2_bOrigWasDF2 && (openjkdf2_restartMode == OPENJKDF2_RESTART_MOTS))
                || (openjkdf2_bOrigWasRunningFromExistingInstall && !openjkdf2_bOrigWasDF2 && (openjkdf2_restartMode == OPENJKDF2_RESTART_DF2))) 
            {
                openjkdf2_bSkipWorkingDirData = 1;
            }
            else {
                openjkdf2_bSkipWorkingDirData = 0;
                chdir(openjkdf2_aOrigCwd);
            }
        }

        if (openjkdf2_restartMode == OPENJKDF2_RESTART_MOTS) {
            Main_bMotsCompat = 1;
            continue;
        }
        else if (openjkdf2_restartMode == OPENJKDF2_RESTART_DF2) {
            Main_bMotsCompat = 0;
            continue;
        }
        else if (openjkdf2_restartMode == OPENJKDF2_RESTART_PATH) {
            continue;
        }
        break;
    }

#ifdef PLATFORM_PHYSFS
    PHYSFS_deinit();
#endif


    return 1;
}


//#include "external/libbacktrace/backtrace.h"



static void full_write(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t ret = write(fd, buf, len);

        if ((ret == -1) && (errno != EINTR))
                break;

        buf += (size_t) ret;
        len -= (size_t) ret;
    }
}

void print_backtrace(void)
{
    static const char start[] = "BACKTRACE:\n----------------------\n";
    static const char end[] = "\n\nPlease report this bug to https://github.com/shinyquagsire23/OpenJKDF2/issues\n"
                              "or email me at mtinc2@gmail.com, thanks!\n"
                              "----------------------\n";

    void *bt[1024];
    int bt_size;
    char **bt_syms;
    int i;

    bt_size = backtrace(bt, 1024);
    bt_syms = backtrace_symbols(bt, bt_size);
    full_write(STDERR_FILENO, start, strlen(start));
    for (i = 1; i < bt_size; i++) {
            size_t len = strlen(bt_syms[i]);
            full_write(STDERR_FILENO, bt_syms[i], len);
            full_write(STDERR_FILENO, "\n", 1);
    }
    full_write(STDERR_FILENO, end, strlen(end));

    char* crash_print = (char*)malloc(1024);
    strcpy(crash_print, start);
    for (i = 1; i < bt_size; i++) {
        strcat(crash_print, bt_syms[i]);
        strcat(crash_print, "\n");
    }
    strcat(crash_print, end);

    FILE* f = fopen("crash.log", "a");
    if (f)
    {
        fwrite(crash_print, 1, strlen(crash_print), f);
        fclose(f);
    }


    free(crash_print);

    free(bt_syms);
}

void crash_handler_basic(int sig) 
{
    print_backtrace();
    signal(SIGSEGV, SIG_DFL); // Pass error to OS; MacOS has nicer crash diagnostics
}

void crash_handler_full(int sig) 
{
    //struct backtrace_state *lbstate;

    //printf ("Backtrace:\n");
    //lbstate = backtrace_create_state (openjkdf2_pExecutablePath, 1, error_callback, NULL);      
    //backtrace_full(lbstate, 0, full_callback, error_callback, 0);
    exit(1);
}

void int_handler(int sig) {
    //exit(0);
    signal(SIGINT, int_handler);
}



int yyparse();
void do_hooks()
{
}

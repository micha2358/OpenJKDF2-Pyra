#include "Window.h"

#include "Win95/stdGdi.h"
#include "Platform/std3D.h"
#include "Main/Main.h"
#include "Main/jkMain.h"
#include "Main/jkGame.h"
#include "Gui/jkGUI.h"
#include "Gui/jkGUIRend.h"
#include "Win95/stdDisplay.h"
#include "World/jkPlayer.h"
#include "Platform/stdControl.h"
#include "stdPlatform.h"
#include "Devices/sithConsole.h"
#include "Platform/wuRegistry.h"
#include "Main/jkQuakeConsole.h"

#include "jk.h"



#include <stdio.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
//#include <stropts.h>

#include "SDL2_helper.h"

#include <string.h>

//#include <GL/glew.h>
//#include <GL/gl.h>
#include "Win95/Video.h"





extern int Window_xPos, Window_yPos;

int Window_xSize = WINDOW_DEFAULT_WIDTH;
int Window_ySize = WINDOW_DEFAULT_HEIGHT;
int Window_screenXSize = WINDOW_DEFAULT_WIDTH;
int Window_screenYSize = WINDOW_DEFAULT_HEIGHT;
int Window_isHiDpi = 0;
int Window_isFullscreen = 0;
int Window_needsRecreate = 0;
int Window_bShouldPopSteamKeyboard = 0;

// Pyra: avoid repeatedly changing SDL relative-mouse state from the hot frame loop.
// SDL_SetRelativeMouseMode() flushes pending mouse motion, so only call it on transitions.
static int Window_bMouseFocus = 1;
static void Window_SetRelativeMouseModeIfNeeded(SDL_bool enabled)
{
    if (SDL_GetRelativeMouseMode() != enabled)
        SDL_SetRelativeMouseMode(enabled);
}

void Window_SetHiDpi(int val)
{
    if (Window_isHiDpi != val)
    {
        Window_isHiDpi = val;

        Window_needsRecreate = 1;
    }

    wuRegistry_SaveBool("Window_isHiDpi", Window_isHiDpi);
}

void Window_SetFullscreen(int val)
{
    if (Window_isFullscreen != val)
    {
        // Reset window when exiting fullscreen
        // TODO: Add settings for these sizes maybe?
        if (Window_isFullscreen && !val) {
            Window_xSize = WINDOW_DEFAULT_WIDTH;
            Window_ySize = WINDOW_DEFAULT_HEIGHT;
            Window_screenXSize = WINDOW_DEFAULT_WIDTH;
            Window_screenYSize = WINDOW_DEFAULT_HEIGHT;
            Window_xPos = SDL_WINDOWPOS_CENTERED;
            Window_yPos = SDL_WINDOWPOS_CENTERED;
        }

        Window_isFullscreen = val;
        Window_needsRecreate = 1;
    }

    wuRegistry_SaveBool("Window_isFullscreen", Window_isFullscreen);
    
}

//static wm_handler Window_ext_handlers[16] = {0};

int Window_AddMsgHandler(WindowHandler_t a1)
{
    int i = 0;

    // Added: no duplicates
    for (i = 0; i < 16; i++)
    {
        if (Window_ext_handlers[i].exists && Window_ext_handlers[i].handler == a1)
            return 1;
    }

    for (i = 0; i < 16; i++)
    {
        if ( !Window_ext_handlers[i].exists )
            break;
    }
    
    // Added: no OOB
    if (i >= 16) return 1;

    Window_ext_handlers[i].handler = a1;
    Window_ext_handlers[i].exists = 1;
    ++g_handler_count;
    return 1;
}

int Window_RemoveMsgHandler(WindowHandler_t a1)
{
    int i = 0;

    // Added: the original would still decrement on missing handlers
    for (i = 0; i < 16; i++)
    {
        if ( Window_ext_handlers[i].handler == a1 )
        {
            Window_ext_handlers[i].handler = 0;
            Window_ext_handlers[i].exists = 0;
            g_handler_count -= 1; // doing g_handler_count-- changes behavior???
            return 1;
        }
    }

    return 1;
}

int Window_AddDialogHwnd(HWND a1)
{
    int v1; // eax

    v1 = g_thing_two_some_dialog_count;
    if ( (unsigned int)g_thing_two_some_dialog_count >= 0x10 )
        return 0;
    Window_aDialogHwnds[g_thing_two_some_dialog_count] = a1;
    g_thing_two_some_dialog_count = v1 + 1;
    return 1;
}

static int dword_855E98 = 0;
static int dword_855DE4 = 0;

int Window_msg_main_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    int handler_count; // ebx
    struct wm_handler *ext_handler; // esi
    DWORD dwProcessId; // [esp+10h] [ebp-8h] BYREF
    LRESULT v10; // [esp+14h] [ebp-4h] BYREF

    switch ( Msg )
    {
        case WM_CREATE:
            g_app_active = 0;
            g_window_active = 0;
            break;
        case WM_DESTROY:
            g_window_not_destroyed = 0;
            Main_Shutdown();
            break;
        case WM_ACTIVATE:
            if ( (uint16_t)wParam == 2 || (uint16_t)wParam == 1 )// WA_ACTIVE or WA_CLICKACTIVE
            {
                g_window_active = 1;
                if ( dword_855E98 )
                {
                    dword_855E98 = 0;
                    if ( Window_setCooperativeLevel )
                        Window_setCooperativeLevel(0);
                }
            }
            else
            {
                if ( dword_855DE4 == 1 && g_window_not_destroyed && g_app_active && !dword_855E98 )
                {
                    dwProcessId = 0;
                    lParam = 1;
                    if ( lParam )
                    {
                    }
                    if ( dwProcessId == lParam )
                    {
                        dword_855E98 = 1;
                        if ( Window_drawAndFlip )
                            Window_drawAndFlip(0);
                    }
                }
                g_window_active = 0;
            }
            break;
        case WM_ACTIVATEAPP:
            g_app_active = wParam != 0;
            break;
        default:
            break;
    }

    if ( !g_app_active || (g_app_suspended = 1, !g_window_active) )
        g_app_suspended = 0;
    handler_count = 0;

    if ( g_handler_count <= 0 )
        return Window_DefaultHandler(hWnd, Msg, wParam, lParam, NULL);

    for ( ext_handler = Window_ext_handlers; !ext_handler->exists || !ext_handler->handler(hWnd, Msg, wParam, lParam, &v10); ++ext_handler )
    {
        if ( ++handler_count >= g_handler_count )
            return Window_DefaultHandler(hWnd, Msg, wParam, lParam, NULL);
    }
    return v10;
}



SDL_Window* displayWindow = NULL;
SDL_Event event;
SDL_GLContext glWindowContext;

int Window_lastXRel = 0;
int Window_lastYRel = 0;
int Window_lastSampleTime = 0;
int Window_lastSampleMs = 0;
int Window_bMouseLeft = 0;
int Window_bMouseRight = 0;
int Window_resized = 0;
int Window_mouseX = 0;
int Window_mouseY = 0;
int Window_mouseWheelX = 0;
int Window_mouseWheelY = 0;
int Window_lastMouseX = 0;
int Window_lastMouseY = 0;
int Window_xPos = SDL_WINDOWPOS_CENTERED;
int Window_yPos = SDL_WINDOWPOS_CENTERED;
int last_jkGame_isDDraw = 0;
#ifdef QUAKE_CONSOLE
int last_jkQuakeConsole_bOpen = 0;
#endif
int Window_menu_mouseX = 0;
int Window_menu_mouseY = 0;

extern int jkGuiBuildMulti_bRendering;

void Window_HandleMouseMove(SDL_MouseMotionEvent *event)
{
    int x = event->x;
    int y = event->y;

    Window_lastMouseX = Window_mouseX;
    Window_lastMouseY = Window_mouseY;

    if (!jkGame_isDDraw)
    {
        // FLEXTODO
        flex_t fX = (flex_t)x;
        flex_t fY = (flex_t)y;

        // Keep 4:3 aspect
        flex_t menu_x = ((flex_t)Window_screenXSize - ((flex_t)Window_screenYSize * (640.0 / 480.0))) / 2.0;
        flex_t menu_w = ((flex_t)Window_screenYSize * (640.0 / 480.0));

        Window_mouseX = (int)(((fX - menu_x) / (flex_t)menu_w) * 640.0);
        Window_mouseY = (int)((fY / (flex_t)Window_screenYSize) * 480.0);
        //printf("%d %d\n", Window_mouseX, Window_mouseY);
    }
    else
    {
        Window_mouseX = x;
        Window_mouseY = y;// - (Window_ySize - 480);
    }

    if (Window_mouseX < 0)
        Window_mouseX = 0;

    if (jkQuakeConsole_bOpen) return; // Hijack all input to console

    uint32_t pos = ((Window_mouseX) & 0xFFFF) | (((Window_mouseY) << 16) & 0xFFFF0000);
    
    Window_lastSampleMs = event->timestamp - Window_lastSampleTime;
    //Window_lastSampleTime = event->timestamp;
    Window_lastXRel += event->xrel;
    Window_lastYRel += event->yrel;

    Window_msg_main_handler(g_hWnd, WM_MOUSEMOVE, 0, pos);
}

/* Pyra: merge consecutive SDL mouse-motion events before running the
 * relatively expensive legacy WM_MOUSEMOVE translation. Relative deltas
 * are preserved exactly and the final absolute position/timestamp wins. */
static void Window_AccumulateMouseMotion(SDL_MouseMotionEvent *dst, const SDL_MouseMotionEvent *src, int *pending)
{
    if (!*pending) {
        *dst = *src;
        *pending = 1;
        return;
    }
    dst->xrel += src->xrel;
    dst->yrel += src->yrel;
    dst->x = src->x;
    dst->y = src->y;
    dst->timestamp = src->timestamp;
    dst->state = src->state;
}

static void Window_FlushMouseMotion(SDL_MouseMotionEvent *motion, int *pending)
{
    if (*pending) {
        Window_HandleMouseMove(motion);
        *pending = 0;
    }
}

int jkCutscene_wasPaused = 0;
int jkGame_wasDDraw = 0;
int Window_bNeedsKeyboardFixed = 0;
void Window_HandleWindowEvent(SDL_Event* event)
{
    switch (event->window.event) 
    {
        case SDL_WINDOWEVENT_SHOWN:
            //printf("Window %d shown", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            //printf("Window %d hidden", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            //printf("Window %d exposed", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            /*printf("Window %d moved to %d,%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);*/
            Window_xPos = event->window.data1;
            Window_yPos = event->window.data2;
            break;
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            int newXSize, newYSize, newScreenXSize, newScreenYSize;
            SDL_GL_GetDrawableSize(displayWindow, &newXSize, &newYSize);
            SDL_GetWindowSize(displayWindow, &newScreenXSize, &newScreenYSize);

            if (newXSize < 640) newXSize = 640;
            if (newYSize < 480) newYSize = 480;

            // SDL can emit both RESIZED and SIZE_CHANGED for one transition.
            // Only run the expensive resolution-fix path when the effective size changed.
            if (Window_xSize != newXSize || Window_ySize != newYSize ||
                Window_screenXSize != newScreenXSize || Window_screenYSize != newScreenYSize)
            {
                Window_xSize = newXSize;
                Window_ySize = newYSize;
                Window_screenXSize = newScreenXSize;
                Window_screenYSize = newScreenYSize;
                Window_resized = 1;
            }
            break;
        }
        case SDL_WINDOWEVENT_MINIMIZED:
            stdPlatform_Printf("Window %d minimized", event->window.windowID);

            // HACK: Cutscene audio gets messed up when multitasking on Android :/
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            stdPlatform_Printf("Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            stdPlatform_Printf("Window %d restored", event->window.windowID);
            
            // HACK: Cutscene audio gets messed up when multitasking on Android :/
            break;
        case SDL_WINDOWEVENT_ENTER:
            Window_bMouseFocus = 1;
            stdPlatform_Printf("Mouse entered window %d\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            Window_bMouseFocus = 0;
            stdPlatform_Printf("Mouse left window %d\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            stdPlatform_Printf("Window %d gained keyboard focus\n", event->window.windowID);
            Window_bNeedsKeyboardFixed = 0;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            stdPlatform_Printf("Window %d lost keyboard focus\n", event->window.windowID);
            if (stdControl_IsSystemKeyboardShowing() && Window_bNeedsKeyboardFixed) {
                stdPlatform_Printf("Fixing keyboard...\n");
                
                SDL_Window* gimmeKeyboard = SDL_CreateWindow("Gimme Keyboard", 20, 20, 20, 20, SDL_WINDOW_KEYBOARD_GRABBED | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);
                SDL_RaiseWindow(gimmeKeyboard);
                SDL_DestroyWindow(gimmeKeyboard);
                SDL_RaiseWindow(displayWindow);

                SDL_MinimizeWindow(displayWindow);
                SDL_RestoreWindow(displayWindow);
                SDL_RaiseWindow(displayWindow);
            }
            break;
        case SDL_WINDOWEVENT_CLOSE:
            //printf("Window %d closed", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            //printf("Window %d is offered a focus", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            //printf("Window %d has a special hit test", event->window.windowID);
            break;
    }
}

int my_kbhit() {
    static const int STDIN = 0;
    static int initialized = 0;

    if (! initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        term.c_lflag &= ~ECHO;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

static char Window_headlessBuffer[256];

void Window_UpdateHeadless()
{
    char buffer[32];
    size_t bytes_read = 0;

    if (my_kbhit() > 0) {
        int fd = STDIN_FILENO;
        bytes_read = read(fd, buffer, sizeof(buffer)-1);
        buffer[bytes_read] = 0;

        for (int i = 0; i < bytes_read; i++)
        {
            if (buffer[i] == '\n' || buffer[i] == '\r') {
                printf("\r> %s\n", Window_headlessBuffer);
                sithConsole_TryCommand(Window_headlessBuffer);
                memset(Window_headlessBuffer, 0, sizeof(Window_headlessBuffer));
                continue;
            }
            else if (buffer[i] == 0x7F && strlen(Window_headlessBuffer)) {
                Window_headlessBuffer[strlen(Window_headlessBuffer)-1] = 0;
                printf("\r> %s ", Window_headlessBuffer);
                continue;
            }
            else if (buffer[i] < ' ' || buffer[i] > '~')
            {
                continue;
            }

            char tmp[2] = {buffer[i], 0};
            strncat(Window_headlessBuffer, tmp, 255);
        }
    }
    
    printf("\r> %s", Window_headlessBuffer);
    //printf("> %x %x %s\n", buffer[0], my_kbhit(), Window_headlessBuffer);
    fflush(stdout);

    if (Window_resized)
    {
        jkMain_FixRes();
        if (!jkGui_SetModeMenu(0))
        {
            stdDisplay_SetMode(0, 0, 0);
            //jkMain_FixRes();
        }

        jkGui_SetModeGame();
        
        Window_resized = 0;
    }
    
    uint32_t sampleTime_now = SDL_GetTicks();
    int sampleTime_roundtrip = sampleTime_now - Window_lastSampleTime;
    //printf("%u\n", sampleTime_roundtrip);
    Window_lastSampleTime = sampleTime_now;

    int menu_framelimit_amt_ms = 6;

    if (!jkGame_isDDraw)
    {

        if (!jkGuiBuildMulti_bRendering) {
            std3D_StartScene();
#ifdef QUAKE_CONSOLE
            jkQuakeConsole_Render();
#endif
            std3D_DrawMenu();
            std3D_EndScene();
            //SDL_GL_SwapWindow(displayWindow);
        }
        else {
#ifdef QUAKE_CONSOLE
            jkQuakeConsole_Render();
#endif
            std3D_DrawMenu();
            //SDL_GL_SwapWindow(displayWindow);
            //menu_framelimit_amt_ms = 64;
        }
    }
    else
    {
        // Save mouse position for menu
        if (jkGame_isDDraw != last_jkGame_isDDraw) {
            Window_menu_mouseX = Window_mouseX;
            Window_menu_mouseY = Window_mouseY;
            Window_lastXRel = 0;
            Window_lastYRel = 0;
        }
    }

    // Pyra: deterministic limiter.  The old feedback controller changed the
    // sleep duration every iteration and introduced avoidable scheduling jitter.
    if (sampleTime_roundtrip < menu_framelimit_amt_ms)
        SDL_Delay((Uint32)(menu_framelimit_amt_ms - sampleTime_roundtrip));

    last_jkGame_isDDraw = jkGame_isDDraw;
    last_jkQuakeConsole_bOpen = jkQuakeConsole_bOpen;
}

void Window_SdlUpdate()
{
    if (Main_bHeadless)
    {
        Window_UpdateHeadless();
        return;
    }

    uint16_t left, right;
    uint32_t pos, msgl, msgr;
    int hasLeft, hasRight;
    SDL_Event event;
    SDL_MouseButtonEvent* mevent;
    SDL_MouseMotionEvent pendingMouseMotion;
    int hasPendingMouseMotion = 0;

    // HACK: Escape key for controllers
    extern int stdControl_bControllerEscapeKey;
    extern int stdControl_bControllerEscapeKey_last;

    while (SDL_PollEvent(&event))
    {
        int bIsOdin = 0;
        int bIsGamepad = 0;

        if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
            const char* name = SDL_JoystickNameForIndex(event.jbutton.which);
            bIsOdin = name && strcmp(name, "Odin Controller") == 0;
            bIsGamepad = SDL_IsGameController(event.jbutton.which);
        }
        if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
            bIsGamepad = 1;
        }

        if (event.type == SDL_MOUSEMOTION) {
            Window_AccumulateMouseMotion(&pendingMouseMotion, &event.motion, &hasPendingMouseMotion);
            continue;
        }
        Window_FlushMouseMotion(&pendingMouseMotion, &hasPendingMouseMotion);

        switch (event.type)
        {
            case SDL_JOYDEVICEADDED: {
                stdControl_bHasJoysticks = 1;
                stdControl_InitSdlJoysticks();
                break;
            }
            case SDL_JOYDEVICEREMOVED: {
                stdControl_InitSdlJoysticks();
                break;
            }

            case SDL_TEXTINPUT:
                for (int i = 0; i < _strlen(event.text.text); i++)
                {
                    Window_msg_main_handler(g_hWnd, WM_CHAR, event.text.text[i], 0);
                }
                break;
            case SDL_WINDOWEVENT:
                Window_HandleWindowEvent(&event);
                break;
            case SDL_KEYDOWN:
                //stdPlatform_Printf("scancode %d\n", event.key.keysym.scancode);
                //handleKey(&event.key.keysym, WM_KEYDOWN, 0x1);
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, event.key.repeat & 0xFFFF);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_PAGEUP)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_PRIOR, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_NEXT, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_LEFT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_LEFT, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_RIGHT, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_UP)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_UP, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_DOWN, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_BACK, event.key.repeat & 0xFFFF);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_BACK, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_DELETE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_DELETE, event.key.repeat & 0xFFFF);
                    //Window_msg_main_handler(g_hWnd, WM_CHAR, VK_DELETE, 0);
                }
                else if (event.key.keysym.sym == SDLK_INSERT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_INSERT, event.key.repeat & 0xFFFF);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_INSERT, 0);
                }
                else if (event.key.keysym.sym == SDLK_RETURN)
                {
                    // HACK apparently Windows buffers these events in some way, but to replicate the behavior in jkGUI we just spam KEYFIRST
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_RETURN, event.key.repeat & 0xFFFF);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_RETURN, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_LSHIFT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_LSHIFT, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_RSHIFT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_RSHIFT, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_TAB)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_TAB, event.key.repeat & 0xFFFF);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_TAB, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.sym == SDLK_END)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_END, event.key.repeat & 0xFFFF);
                    //Window_msg_main_handler(g_hWnd, WM_CHAR, 0x23, 0);
                }
                else if (event.key.keysym.sym == SDLK_HOME)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_HOME, event.key.repeat & 0xFFFF);
                    //Window_msg_main_handler(g_hWnd, WM_CHAR, 0x24, 0);
                }
                else if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_OEM_3, event.key.repeat & 0xFFFF);
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, 0);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, event.key.repeat & 0xFFFF);
                }

                //if (!event.key.repeat)
                //    stdControl_SetSDLKeydown(event.key.keysym.scancode, 1, event.key.timestamp);
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_ESCAPE, 0);
                }
                else if (event.key.keysym.sym == SDLK_PAGEUP)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_PRIOR, 0);
                }
                else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_NEXT, 0);
                }
                else if (event.key.keysym.sym == SDLK_LEFT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_LEFT, 0);
                }
                else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_RIGHT, 0);
                }
                else if (event.key.keysym.sym == SDLK_UP)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_UP, 0);
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_DOWN, 0);
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_BACK, 0);
                }
                else if (event.key.keysym.sym == SDLK_DELETE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_DELETE, 0);
                }
                else if (event.key.keysym.sym == SDLK_INSERT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_INSERT, 0);
                }
                else if (event.key.keysym.sym == SDLK_RETURN)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_RETURN, 0); // 0xB?
                }
                else if (event.key.keysym.sym == SDLK_LSHIFT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_LSHIFT, 0);
                }
                else if (event.key.keysym.sym == SDLK_RSHIFT)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_RSHIFT, 0);
                }
                else if (event.key.keysym.sym == SDLK_TAB)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_TAB, 0);
                }
                else if (event.key.keysym.sym == SDLK_END)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_END, 0);
                }
                else if (event.key.keysym.sym == SDLK_HOME)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_HOME, 0);
                }
                else if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_OEM_3, 0);
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
                    Window_msg_main_handler(g_hWnd, WM_KEYUP, VK_ESCAPE, 0);
                }
                //handleKey(&event.key.keysym, WM_KEYUP, 0xc0000001);

                if (jkQuakeConsole_bOpen) break; // Hijack all input to console

                stdControl_SetSDLKeydown(event.key.keysym.scancode, 0, event.key.timestamp);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:

                mevent = (SDL_MouseButtonEvent*)&event;
                left = 0;
                right = 0;
                hasLeft = 0;
                hasRight = 0;
                if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    left = (mevent->button == SDL_BUTTON_LEFT ? 1 : 0);
                    right = (mevent->button == SDL_BUTTON_RIGHT ? 2 : 0);
                    
                    if (left)
                        hasLeft = 1;
                    if (right)
                        hasRight = 1;
                }
                else if (event.type == SDL_MOUSEBUTTONUP)
                {
                    left = (mevent->button == SDL_BUTTON_LEFT ? 0 : 1);
                    right = (mevent->button == SDL_BUTTON_RIGHT ? 0 : 2);
                    
                    if (!left)
                        hasLeft = 1;
                    if (!right)
                        hasRight = 1;
                }
                
                if (hasLeft)
                    Window_bMouseLeft = left;
                if (hasRight)
                    Window_bMouseRight = right;

                Window_mouseX = mevent->x;
                Window_mouseY = mevent->y;// - (Window_ySize - 480);

                pos = ((Window_mouseX) & 0xFFFF) | (((Window_mouseY) << 16) & 0xFFFF0000);
                msgl = (event.type == SDL_MOUSEBUTTONDOWN ? WM_LBUTTONDOWN : WM_LBUTTONUP);
                msgr = (event.type == SDL_MOUSEBUTTONDOWN ? WM_RBUTTONDOWN : WM_RBUTTONUP);

                if (jkQuakeConsole_bOpen) break; // Hijack all input to console
                
                if (hasLeft)
                    Window_msg_main_handler(g_hWnd, msgl, left | right, pos);
                if (hasRight)
                    Window_msg_main_handler(g_hWnd, msgr, left | right, pos);

                //stdControl_SetKeydown(KEY_MOUSE_B1, Window_bMouseLeft, mevent->timestamp);
                //stdControl_SetKeydown(KEY_MOUSE_B2, Window_bMouseRight, mevent->timestamp);

                break;
            case SDL_MOUSEWHEEL:
                Window_mouseWheelY = event.wheel.y;
                Window_mouseWheelX = event.wheel.x;

                if (jkQuakeConsole_bOpen) break; // Hijack all input to console
                break;

            // HACK: Escape key for controllers
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                if (!bIsGamepad) {
                    //stdPlatform_Printf("button %d, %d\n", event.jbutton.button, event.jbutton.state);
                }
                if (bIsOdin && !bIsGamepad && (event.jbutton.button == 6 || event.jbutton.button == 4)) {
                    stdControl_bControllerEscapeKey = (event.jbutton.state == SDL_PRESSED);
                }
                else if (!bIsGamepad && jkCutscene_isRendering && event.type == SDL_JOYBUTTONDOWN && event.jbutton.button == 3) { // y
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_SPACE, 0);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_SPACE, 0);
                }
                else if (!bIsGamepad && jkCutscene_isRendering  && event.type == SDL_JOYBUTTONDOWN&& event.jbutton.button == 2) { // x
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_SPACE, 0);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_SPACE, 0);
                }
                else if (!bIsGamepad && jkCutscene_isRendering && event.type == SDL_JOYBUTTONDOWN && event.jbutton.button == 1) { // b
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, 0);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, 0);
                }
                else if (!bIsGamepad && jkCutscene_isRendering && event.type == SDL_JOYBUTTONDOWN && event.jbutton.button == 0) { // a
                    Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, 0);
                    Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, 0);
                }
                break;

            case SDL_JOYAXISMOTION:
                if (event.jaxis.which == 0) {
                    //stdPlatform_Printf("axis %d, %d\n", event.jaxis.axis, event.jaxis.value);
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                if (bIsGamepad) {
                    //stdPlatform_Printf("gpad button %d, %d\n", event.cbutton.button, event.cbutton.state);
                    if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START || event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                        stdControl_bControllerEscapeKey = (event.cbutton.state == SDL_PRESSED);
                    }
                    else if (jkCutscene_isRendering && event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_Y) { // y
                        Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_SPACE, 0);
                        Window_msg_main_handler(g_hWnd, WM_CHAR, VK_SPACE, 0);
                    }
                    else if (jkCutscene_isRendering  && event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_X) { // x
                        Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_SPACE, 0);
                        Window_msg_main_handler(g_hWnd, WM_CHAR, VK_SPACE, 0);
                    }
                    else if (jkCutscene_isRendering && event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_B) { // b
                        Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, 0);
                        Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, 0);
                    }
                    else if (jkCutscene_isRendering && event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_A) { // a
                        Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, 0);
                        Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, 0);
                    }
                }
                break;
            case SDL_CONTROLLERAXISMOTION:
                //stdPlatform_Printf("Controller %d Axis %d moved to %d\n", 
                //       event.caxis.which, event.caxis.axis, event.caxis.value);
                break;

            case SDL_QUIT:
                stdPlatform_Printf("Quit!\n");

                // Added
                if (jkPlayer_bHasLoadedSettingsOnce) {
                    jkPlayer_WriteConf(jkPlayer_playerShortName);
                }
                
                exit(-1);
                break;
            default:
                break;
        }
    }

    // HACK: Escape key for controllers
    if (stdControl_bControllerEscapeKey && !stdControl_bControllerEscapeKey_last) {
        Window_msg_main_handler(g_hWnd, WM_KEYFIRST, VK_ESCAPE, 0);
        Window_msg_main_handler(g_hWnd, WM_CHAR, VK_ESCAPE, 0);
    }
    stdControl_bControllerEscapeKey_last = stdControl_bControllerEscapeKey;
    
    Window_FlushMouseMotion(&pendingMouseMotion, &hasPendingMouseMotion);

    if (Window_resized)
    {
        jkMain_FixRes();
        if (!jkGui_SetModeMenu(0))
        {
            stdDisplay_SetMode(0, 0, 0);
            //jkMain_FixRes();
        }
        
        Window_resized = 0;
    }
    
    uint32_t sampleTime_now = SDL_GetTicks();
    int sampleTime_roundtrip = sampleTime_now - Window_lastSampleTime;
    //printf("%u\n", sampleTime_roundtrip);
    Window_lastSampleTime = sampleTime_now;

    static int jkPlayer_enableVsync_last = 0;
    int menu_framelimit_amt_ms = 16;

    if (jkPlayer_enableVsync_last != jkPlayer_enableVsync)
    {
        SDL_GL_SetSwapInterval(jkPlayer_enableVsync);
    }

    if (!jkGame_isDDraw)
    {
        // Restore menu mouse position
        if (jkGame_isDDraw != last_jkGame_isDDraw) {
            SDL_WarpMouseInWindow(displayWindow, Window_menu_mouseX, Window_menu_mouseY);
        }

        Window_SetRelativeMouseModeIfNeeded(SDL_FALSE);

        if (!jkGuiBuildMulti_bRendering) {
            std3D_StartScene();
#ifdef QUAKE_CONSOLE
            jkQuakeConsole_Render();
#endif
            std3D_DrawMenu();
            std3D_EndScene();
            SDL_GL_SwapWindow(displayWindow);
        }
        else {
#ifdef QUAKE_CONSOLE
            jkQuakeConsole_Render();
#endif
            std3D_DrawMenu();
            SDL_GL_SwapWindow(displayWindow);
            //menu_framelimit_amt_ms = 64;
        }

        if (Window_needsRecreate) {
            std3D_PurgeEntireTextureCache();
            Window_RecreateSDL2Window();
        }
        
        // Keep menu near 60 FPS without the old oscillating feedback delay.
        if (sampleTime_roundtrip < menu_framelimit_amt_ms)
            SDL_Delay((Uint32)(menu_framelimit_amt_ms - sampleTime_roundtrip));
    }
    else
    {
        // Save mouse position for menu
        if (jkGame_isDDraw != last_jkGame_isDDraw) {
            Window_menu_mouseX = Window_mouseX;
            Window_menu_mouseY = Window_mouseY;
            Window_lastXRel = 0;
            Window_lastYRel = 0;
        }

#ifdef QUAKE_CONSOLE

        if (jkQuakeConsole_bOpen && jkQuakeConsole_bOpen != last_jkQuakeConsole_bOpen) {
            SDL_WarpMouseInWindow(displayWindow, Window_menu_mouseX, Window_menu_mouseY);
        }
        else if (!jkQuakeConsole_bOpen && jkQuakeConsole_bOpen != last_jkQuakeConsole_bOpen) {
            Window_menu_mouseX = Window_mouseX;
            Window_menu_mouseY = Window_mouseY;
            Window_lastXRel = 0;
            Window_lastYRel = 0;
        }

        // Mouse focus is maintained by SDL window enter/leave events. Avoid querying
        // window flags and re-applying relative mode on every gameplay frame.
        Window_SetRelativeMouseModeIfNeeded((!jkQuakeConsole_bOpen && Window_bMouseFocus)
                                                ? SDL_TRUE : SDL_FALSE);
#endif
    }

    jkPlayer_enableVsync_last = jkPlayer_enableVsync;

    last_jkGame_isDDraw = jkGame_isDDraw;
#ifdef QUAKE_CONSOLE
    last_jkQuakeConsole_bOpen = jkQuakeConsole_bOpen;
#endif
}

void Window_SdlVblank()
{
    if (Main_bHeadless) return;

    //static uint32_t roundtrip = 0;
    //uint32_t before = stdPlatform_GetTimeMsec();
    SDL_GL_SwapWindow(displayWindow);
    //uint32_t after = stdPlatform_GetTimeMsec();
    //printf("%u %u\n", after-before, before-roundtrip);

    //roundtrip = before;

    if (Window_needsRecreate)
        Window_RecreateSDL2Window();

}


void Window_RecreateSDL2Window()
{

    if (Main_bHeadless) return;

    stdPlatform_Printf("Recreating SDL2 Window!\n");
    Window_needsRecreate = 0;

    if (displayWindow) {
        std3D_FreeResources();
        SDL_GL_DeleteContext(glWindowContext);
        SDL_DestroyWindow(displayWindow);
    }

    // HACK: side-step the json stuff
    if (Window_bShouldPopSteamKeyboard) {
        Window_isFullscreen = 1;
        Window_isHiDpi = 1;
    }

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    if (displayWindow) {
        flags = SDL_GetWindowFlags(displayWindow);
        //std3D_FreeResources();
        //SDL_GL_DeleteContext(glWindowContext);
        //SDL_DestroyWindow(displayWindow);

        flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    }


    if (Window_isHiDpi)
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    else
        flags &= ~SDL_WINDOW_ALLOW_HIGHDPI;

    if (Window_isFullscreen) {
        //flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else {
        //flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
    }



    displayWindow = SDL_CreateWindow(Window_isHiDpi ? "OpenJKDF2 HiDPI" : "OpenJKDF2", Window_xPos, Window_yPos, Window_screenXSize, Window_screenYSize, flags);
    if (!displayWindow) {
        char errtmp[256];
        snprintf(errtmp, 256, "!! Failed to create SDL2 window !!\n%s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errtmp, NULL);
        exit (-1);
    }
    //SDL_SetRenderDrawBlendMode(displayRenderer, SDL_BLENDMODE_BLEND);

#if defined(MACOS) && defined(__aarch64__)
    //SDL_FixWindowMacOS(displayWindow);
#endif

    if (Window_isFullscreen) {
        SDL_SetWindowFullscreen(displayWindow, SDL_WINDOW_FULLSCREEN);
    }
    else {
        SDL_SetWindowFullscreen(displayWindow, 0);
    }
    SDL_RaiseWindow(displayWindow);
    Window_bMouseFocus = (SDL_GetWindowFlags(displayWindow) & SDL_WINDOW_MOUSE_FOCUS) != 0;

    glWindowContext = SDL_GL_CreateContext(displayWindow);
    
    // Desktop-only retries; a Pyra build must remain GLES2.

    if (glWindowContext == NULL)
    {
        char errtmp[256];
        snprintf(errtmp, 256, "!! Failed to initialize SDL OpenGL context !!\n%s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errtmp, NULL);
        exit(-1);
    }

    SDL_GL_MakeCurrent(displayWindow, glWindowContext);
    SDL_GL_SetSwapInterval(jkPlayer_enableVsync); // Disable vsync
    SDL_StartTextInput();

    SDL_GL_GetDrawableSize(displayWindow, &Window_xSize, &Window_ySize);
    SDL_GetWindowSize(displayWindow, &Window_screenXSize, &Window_screenYSize);

    Window_resized = 1;
}

void Window_Main_Loop()
{
    jkMain_GuiAdvance(); // TODO needed?
    Window_msg_main_handler(g_hWnd, WM_PAINT, 0, 0);

    //Window_SdlUpdate();
}


static void Window_ConfigurePyraPowerVR(void)
{
    /*
     * Force SDL2 to use X11 + EGL + PowerVR GLES2.
     * This avoids SDL/X11 loading desktop libGL/GLX, which would pull in
     * Mesa/swrast and create llvmpipe threads alongside the PowerVR stack.
     */
    SDL_SetHint(SDL_HINT_VIDEO_X11_FORCE_EGL, "1");

    SDL_setenv("SDL_VIDEO_GLES2", "1", 1);
    SDL_setenv("SDL_OPENGL_ES_DRIVER", "1", 1);
    SDL_setenv(
        "SDL_VIDEO_GL_DRIVER",
        "/opt/omap5-sgx-ddk-um-linux/lib/libGLESv2.so",
        1
    );
    SDL_setenv(
        "SDL_VIDEO_EGL_DRIVER",
        "/opt/omap5-sgx-ddk-um-linux/lib/libEGL.so",
        1
    );

    printf("Pyra: forcing X11/EGL + PowerVR GLES2\\n");
}

int Window_Main_Linux(int argc, char** argv)
{
    char cmdLine[1024];
    int result;

    // Init SDL
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_SetHint(SDL_HINT_APP_NAME, "OpenJKDF2");

    Window_ConfigurePyraPowerVR();


SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE | SDL_INIT_GAMECONTROLLER);

    
    if ((SDL_GetHintBoolean("SteamClientLaunch", 0) || SDL_GetHintBoolean("SteamOS", 0) || SDL_GetHintBoolean("SteamDeck", 0)) && SDL_GetHintBoolean("SteamGamepadUI", 0)) {
        Window_bShouldPopSteamKeyboard = 1;
        Window_isFullscreen = 1;
        Window_isHiDpi = 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    Window_RecreateSDL2Window();
#if !defined(TARGET_ANDROID) && !defined(ARCH_WASM) && !defined(RENDER_GL11) && !defined(TARGET_GLES2)
    glewInit();
#endif
    
    //SDL_RenderClear(displayRenderer);
    //SDL_RenderPresent(displayRenderer);
    
    
    strcpy(cmdLine, "");
    
    g_handler_count = 0;
    g_thing_two_some_dialog_count = 0;
    g_should_exit = 0;
    g_window_not_destroyed = 0;
    g_hInstance = 0;//hInstance;
    g_nShowCmd = 0;//nShowCmd;
    
    for (int i = 1; i < argc; i++)
    {
        strcat(cmdLine, argv[i]);
        strcat(cmdLine, " ");
    }
    
    result = Main_Startup(cmdLine);

    int fullscreen = wuRegistry_GetBool("Window_isFullscreen", 1);
    int hidpi = wuRegistry_GetBool("Window_isHiDpi", 0);
    Window_SetFullscreen(fullscreen);
    Window_SetHiDpi(hidpi);
    Window_RecreateSDL2Window();

    if (!result) return result;

    if (Main_bHeadless)
    {
        if (displayWindow) {
            std3D_FreeResources();
            SDL_GL_DeleteContext(glWindowContext);
            SDL_DestroyWindow(displayWindow);
        }
    }

    g_window_not_destroyed = 1;
    
    Window_msg_main_handler(g_hWnd, 0x1, 0, 0); // WM_CREATE
    Window_msg_main_handler(g_hWnd, 0x6, 2, 0); // WM_ACTIVATE
    Window_msg_main_handler(g_hWnd, 0x1C, 1, 0); // WM_ACTIVATEAPP
    Window_msg_main_handler(g_hWnd, 0x18, 0, 0); // WM_SHOWWINDOW
    Window_msg_main_handler(g_hWnd, WM_PAINT, 0, 0);


    while (1)
    {
        Window_Main_Loop();
        if (g_should_exit) break;
    }

    // Added
    if (jkPlayer_bHasLoadedSettingsOnce) {
        jkPlayer_WriteConf(jkPlayer_playerShortName);
    }

    Main_Shutdown();
    return 1;
}

int Window_Main(HINSTANCE hInstance, int a2, char *lpCmdLine, int nShowCmd, LPCSTR lpWindowName)
{
    int result;

    g_handler_count = 0;
    g_thing_two_some_dialog_count = 0;
    g_should_exit = 0;
    g_window_not_destroyed = 0;
    g_hInstance = hInstance;
    g_nShowCmd = nShowCmd;
    result = 1;
    return result;
}

int Window_ShowCursorUnwindowed(int a1)
{
    return stdControl_ShowCursor(a1);
}

int Window_DefaultHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void* unused)
{
    return 0;
}

int Window_MessageLoop()
{
    // Added: controller menuing
    jkGuiRend_UpdateController();

    jkMain_GuiAdvance();
    Window_msg_main_handler(g_hWnd, WM_PAINT, 0, 0);
    
    //Window_SdlUpdate();
    return 0;
}


void Window_SetDrawHandlers(WindowDrawHandler_t a1, WindowDrawHandler_t a2)
{
    Window_drawAndFlip = a1;
    Window_setCooperativeLevel = a2;
}

void Window_GetDrawHandlers(WindowDrawHandler_t *a1, WindowDrawHandler_t *a2)
{
    *a1 = Window_drawAndFlip;
    *a2 = Window_setCooperativeLevel;
}

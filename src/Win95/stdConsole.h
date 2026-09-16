#ifndef _STDCONSOLE_H
#define _STDCONSOLE_H

#include "types.h"
#include "globals.h"

#define stdConsole_Startup_ADDR (0x004277B0)
#define stdConsole_Shutdown_ADDR (0x00427880)
#define stdConsole_New_ADDR (0x00427890)
#define stdConsole_Free_ADDR (0x004279C0)
#define stdConsole_SetCursorPos_ADDR (0x004279E0)
#define stdConsole_GetCursorPos_ADDR (0x00427A10)
#define stdConsole_ToggleCursor_ADDR (0x00427A40)
#define stdConsole_GetTextAttribute_ADDR (0x00427AA0)
#define stdConsole_SetTextAttribute_ADDR (0x00427AC0)
#define stdConsole_Flush_ADDR (0x00427AE0)
#define stdConsole_Clear_ADDR (0x00427AF0)
#define stdConsole_Reset_ADDR (0x00427B60)
#define stdConsole_Putc_ADDR (0x00427BB0)
#define stdConsole_Puts_ADDR (0x00427BF0)
#define stdConsole_ClearBuf_ADDR (0x00427C40)
#define stdConsole_ClearBuf2_ADDR (0x00427C80)
#define stdConsole_WriteBorderMaybe_ADDR (0x00427CB0)
#define stdConsole_WriteBorderMaybe2_ADDR (0x00428360)
#define stdConsole_WriteBorderMaybe3_ADDR (0x004284D0)
#define stdConsole_WriteBorderMaybe4_ADDR (0x00428680)

int stdConsole_Startup(LPCSTR lpConsoleTitle, uint32_t dwWriteCoord, int a3);
int stdConsole_Shutdown();

#endif // _STDCONSOLE_H

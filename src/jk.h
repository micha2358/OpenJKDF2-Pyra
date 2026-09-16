#ifndef _OPENJKDF2_JK_H
#define _OPENJKDF2_JK_H

#include "engine_config.h"
#include "types.h"
#include "stdPlatform.h"
#include <stdio.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef __cplusplus
#include "Cog/sithCogParse.h"
#endif

#define WinMain_ADDR (0x41EBD0)

#define VM_VAR(name, type, ptr) \
    type* name ## _ptr = (type*)ptr;
#define VM_VAR_DECL(name, type) extern type* name ## _ptr;

#ifdef __cplusplus
extern "C" {
#endif

//static void (*jk_main)(uint32_t a, uint32_t b, char* c, int d, char* e) = (void*)0x50E750;

char* _strcpy(char *dst, const char *src);
int _memcmp(const void* str1, const void* str2, size_t count) PURE_FUNC;
void* _memset(void* ptr, int val, size_t num);
#define _memset_inline _memset
void* _memset32(void* ptr, uint32_t val, size_t num);
int _sscanf(const char * s, const char * format, ...);
int _sprintf(char * s, const char * format, ...);
int _rand();
char* _strncpy(char* dst, const char* src, size_t num);
void* _memcpy(void* dst, const void* src, size_t len);
double _atof(const char* str);
int _atoi(const char* str);
uint32_t _atol(const char* s);
size_t _fwrite(const void * a, size_t b, size_t c, FILE * d);
int _fputs(const char * a, FILE * b);
void jk_exit(int a);
int jk_printf(const char* fmt, ...);
int _printf(const char* fmt, ...);
void* _malloc(size_t a);
void _free(void* a);
wchar_t* _wcsncpy(wchar_t *a1, const wchar_t *a2, size_t a3);
void _strtolower(char* str);
void _qsort(void *a, size_t b, size_t c, int (__cdecl *d)(const void *, const void *));
char* _strchr(char * a, char b);
char* _strrchr(char * a, char b);
char* _strtok(char * a, const char * b);
char* _strncat(char* a, const char* b, size_t c);
size_t _strspn(const char* a, const char* b);
const char* _strpbrk(const char* a, const char* b);
size_t _wcslen(const wchar_t * a);
size_t __wcslen(const wchar_t * strarg);
int jk_snwprintf(wchar_t *a1, size_t a2, const wchar_t *fmt, ...);
wchar_t* _wcscpy(wchar_t * a, const wchar_t *b);
int jk_MessageBeep(int a);
int __strcmpi(const char *a, const char *b);
int __strnicmp(const char *a, const char *b, size_t c);
char __tolower(char a);
int msvc_sub_512D30(int a, int b);
int jk_MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
int stdGdi_GetHwnd();
void jk_PostMessageA();
void jk_GetCursorPos(LPPOINT lpPoint);
int jk_GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase);
void jk_BeginPaint(int a, struct tagPAINTSTRUCT * lpPaint);
int jk_vsnwprintf(wchar_t * a, size_t b, const wchar_t *fmt, va_list list);
void jk_EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint);
HINSTANCE stdGdi_GetHInstance();
int jk_LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
void jk_SetCursor(HCURSOR hCursor);
void jk_InvalidateRect(HWND hWnd, const RECT *lpRect, BOOL bErase);
void jk_ChangeDisplaySettingsA(int a, int b);
uint32_t jk_DirectDrawEnumerateA(void* a, void** b);
uint32_t jk_DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);
uint32_t jk_DirectSoundCreate(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
uint32_t jk_DirectPlayLobbyCreateA(GUID *lpGUID, void**, IUnknown*, LPVOID, int);
uint32_t jk_DirectInputCreateA(int a, int b, void** c, LPUNKNOWN d);
uint32_t jk_CreateFileA();
uint32_t jk_CreateFileMappingA();
void* jk_LocalAlloc();
uint32_t jk_MapViewOfFile();
void jk_UnmapViewOfFile(LPCVOID lpBaseAddress);
void jk_CloseHandle(HANDLE hObject);
uint32_t jk_GetDesktopWindow();
uint32_t jk_GetDC(HWND hWnd);
uint32_t jk_GetDeviceCaps(HDC hdc, int index);
uint32_t jk_WinExec(const char* a, int b);
int _string_modify_idk(int c);
void jk_ReleaseDC(HWND hWnd, HDC hDC);
void jk_SetFocus(HWND hWnd);
void jk_SetActiveWindow(HWND hWnd);
void jk_ShowCursor(int a);
void jk_ValidateRect(HWND hWnd, const RECT *lpRect);
int __isspace(int a);
wchar_t* __wcscat(wchar_t *, const wchar_t *);
wchar_t* __wcschr(const wchar_t *, wchar_t);
wchar_t* __wcsncpy(wchar_t *, const wchar_t *, size_t);
wchar_t* __wcsrchr(const wchar_t *, wchar_t);
int __snprintf(char *, size_t, const char *, ...);
int __vsnprintf(char *a1, size_t a2, const char *fmt, va_list args);
char* _strstr(const char* a, const char* b);

long jk_wcstol(const wchar_t * nptr, wchar_t ** endptr, int base);
int _strlen(const char *str);
char* _strcat(char* str, const char* concat);
int _strcmp(const char* s1, const char* s2);
int _strncmp(const char *s1, const char *s2, size_t n);
int __wcscmp(const wchar_t *a, const wchar_t *b);
int __wcsicmp(const wchar_t *a, const wchar_t *b);

flex_t _frand();

void jk_init();
int _iswspace(int a);
void jk_fatal();

#define OPENJKDF2_WARN_NULL_PRINT(var) jk_printf("OpenJKDF2 WARN: " #var " is NULL in %s!\n", __func__)
#define OPENJKDF2_WARN_NULL_NO_RETURN(var) if(!var) { OPENJKDF2_WARN_NULL_PRINT(var); }
#define OPENJKDF2_WARN_NULL_AND_RETURN(var) if(!var) { OPENJKDF2_WARN_NULL_PRINT(var); return; }

#ifdef __cplusplus
}
#endif

#endif // _OPENJKDF2_JK_H

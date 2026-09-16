#include "jk.h"

#include "types.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
//#include <wchar.h>






#include "General/stdString.h"
#include "Platform/stdControl.h"

// Imports


#ifndef LONG_MAX
#define LONG_MAX ((long)(~0UL>>1))
#endif

#ifndef LONG_MIN
#define LONG_MIN (~LONG_MAX)
#endif

long jk_wcstol(const wchar_t */*restrict*/ nptr, wchar_t **/*restrict*/ endptr, int base)
{
    const wchar_t *p = nptr, *endp;
    int is_neg = 0, overflow = 0;
    /* Need unsigned so (-LONG_MIN) can fit in these: */
    unsigned long n = 0UL, cutoff;
    int cutlim;
    if (base < 0 || base == 1 || base > 36) {
#ifdef EINVAL /* errno value defined by POSIX */
        //errno = EINVAL;
#endif
        return 0L;
    }
    endp = nptr;
    while (isspace(*p))
        p++;
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        is_neg = 1, p++;
    }
    if (*p == '0') {
        p++;
        /* For strtol(" 0xZ", &endptr, 16), endptr should point to 'x';
         * pointing to ' ' or '0' is non-compliant.
         * (Many implementations do this wrong.) */
        endp = p;
        if (base == 16 && (*p == 'X' || *p == 'x')) {
            p++;
        } else if (base == 0) {
            if (*p == 'X' || *p == 'x') {
                base = 16, p++;
            } else {
                base = 8;
            }
        }
    } else if (base == 0) {
        base = 10;
    }
    cutoff = (is_neg) ? -(LONG_MIN / base) : LONG_MAX / base;
    cutlim = (is_neg) ? -(LONG_MIN % base) : LONG_MAX % base;
    while (1) {
        int c;
        if (*p >= 'A')
            c = ((*p - 'A') & (~('a' ^ 'A'))) + 10;
        else if (*p <= '9')
            c = *p - '0';
        else
            break;
        if (c < 0 || c >= base) break;
        endp = ++p;
        if (overflow) {
            /* endptr should go forward and point to the non-digit character
             * (of the given base); required by ANSI standard. */
            if (endptr) continue;
            break;
        }
        if (n > cutoff || (n == cutoff && c > cutlim)) {
            overflow = 1; continue;
        }
        n = n * base + c;
    }
    if (endptr) *endptr = (wchar_t *)endp;
    if (overflow) {
        /*errno = ERANGE;*/ return ((is_neg) ? LONG_MIN : LONG_MAX);
    }
    return (long)((is_neg) ? -n : n);
}

int _memcmp (const void* str1, const void* str2, size_t count)
{
  /*register*/ const unsigned char *s1 = (const unsigned char*)str1;
  /*register*/ const unsigned char *s2 = (const unsigned char*)str2;

  while (count-- > 0)
    {
      if (*s1++ != *s2++)
	  return s1[-1] < s2[-1] ? -1 : 1;
    }
  return 0;
}

int _strlen(const char *str)
{
    int len;
    for (len = 0; str[len]; len++);
    return len;
}

char* _strcpy(char *dst, const char *src)
{
    if (!dst) return NULL;
    if (!src) return NULL;

    char *tmp = dst;
    while((*(dst++) = *(src++)));
    return tmp;
}

char* _strcat(char* str, const char* concat)
{
    _strcpy(str+_strlen(str), concat);
    return str;
}

void* _memset(void* ptr, int val, size_t num)
{
    size_t i;
    for (i = 0; i < num; i++)
    {
        *(uint8_t*)((char*)ptr+i) = val;
    }
    return ptr;
}


void* _memset32(void* ptr, uint32_t val, size_t num)
{
    int i;
    for (i = 0; i < num; i++)
    {
        *(uint32_t*)((char*)ptr+(i*sizeof(uint32_t))) = val;
    }
    return ptr;
}

int _strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2))
        s1++, s2++;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int _strncmp(const char *s1, const char *s2, size_t n)
{
  unsigned char u1, u2;
  while (n-- > 0)
    {
      u1 = (unsigned char) *s1++;
      u2 = (unsigned char) *s2++;
      if (u1 != u2)
        return u1 - u2;
      if (u1 == '\0')
        return 0;
    }
  return 0;
}


flex_t _frand()
{
    return (flex_t)(_rand() & 0x7FFF) * 0.000030518509;
}

int __wcscmp(const wchar_t *a, const wchar_t *b)
{
    int ca, cb;

    do 
    {
        ca = (uint16_t) *a++;
        cb = (uint16_t) *b++;
    }
    while (ca == cb && ca != '\0');

    return ca - cb;
}

int __wcsicmp(const wchar_t *a, const wchar_t *b)
{
    int ca, cb;

    do 
    {
        ca = (uint16_t) *a++;
        cb = (uint16_t) *b++;
        ca = __tolower(ca);
        cb = __tolower(cb);
    }
    while (ca == cb && ca != '\0');

    return ca - cb;
}

void jk_fatal()
{
    assert(0);
}

void jk_init()
{
}

#include <ctype.h>
#include "wprintf.h"

int _sscanf(const char * s, const char * format, ...)
{
    va_list args;
    va_start (args, format);
    int ret = vsscanf (s, format, args);
    va_end (args);
    return ret;
}

int _sprintf(char * s, const char * format, ...)
{
    va_list args;
    va_start (args, format);
    int ret = vsnprintf (s, 0x7FFF, format, args);
    va_end (args);
    return ret;
}

int _rand()
{
    return rand();
}

char* _strncpy(char* dst, const char* src, size_t num)
{
    return strncpy(dst, src, num);
}

void* _memcpy(void* dst, const void* src, size_t len)
{
    return memcpy(dst, src, len);
}

double _atof(const char* str)
{
    return atof(str);
}

int _atoi(const char* str)
{
    return atoi(str);
}

uint32_t _atol(const char* s)
{
    return atol(s);
}

size_t _fwrite(const void * a, size_t b, size_t c, FILE * d)
{
    return fwrite(a,b,c,d);
}

int _fputs(const char * a, FILE * b)
{
    return fputs(a, b);
}

void jk_exit(int a)
{
    exit(a);
}


int jk_printf(const char* fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    int ret = vprintf(fmt, args);
    va_end (args);
    return ret;
}

int _printf(const char* fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    int ret = vprintf(fmt, args);
    va_end (args);
    return ret;
}

void* _malloc(size_t a)
{
    return malloc(a);
}

void _free(void* a)
{
    free(a);
}

wchar_t* _wcsncpy(wchar_t *s1, const wchar_t *s2, size_t n)
{
    wchar_t *ret = s1;
    for ( ; n; n--) if (!(*s1++ = *s2++)) break;
    for ( ; n; n--) *s1++ = 0;
    return ret;
}

void _strtolower(char* str)
{
    for(int i = 0; str[i]; i++){
      str[i] = tolower(str[i]);
    }
}

void _qsort(void *a, size_t b, size_t c, int (__cdecl *d)(const void *, const void *))
{
    qsort(a,b,c,d);
}

char* _strchr(char * a, char b)
{
    return strchr(a,b);
}

char* _strrchr(char * a, char b)
{
    return strrchr(a,b);
}

char* _strtok(char * a, const char * b)
{
    // Use strtok_r with our own private save pointer instead of strtok(). strtok()
    // keeps a single global state pointer, and some C libraries call strtok()
    // internally from unrelated functions -- notably KallistiOS normalizes paths
    // with strtok() inside fopen()/open() (fs_normalize_path, realpath). The engine
    // routinely tokenizes a buffer and opens files mid-tokenization (e.g. parsing a
    // .3do/.jkl line while loading a referenced material), so sharing the global
    // strtok state with libc corrupts the in-progress parse. A private save pointer
    // makes our tokenization immune to any library strtok() use.
    static char* _strtok_saveptr = NULL;
    return strtok_r(a, b, &_strtok_saveptr);
}

char* _strncat(char* a, const char* b, size_t c)
{
    return strncat(a,b,c);
}

size_t _strspn(const char* a, const char* b)
{
    return strspn(a,b);
}

const char* _strpbrk(const char* a, const char* b)
{
    return strpbrk(a,b);
}

size_t _wcslen(const wchar_t * str)
{
    int len;
    for (len = 0; str[len]; len++);
    return len;
}

char* _strstr(const char* a, const char* b)
{
    return strstr((char*)a,(char*)b);
}

int jk_snwprintf(wchar_t *a1, size_t a2, const wchar_t *fmt, ...)
{

    va_list args;
    va_start (args, fmt);
    int ret = vsnwprintf_(a1, a2, fmt, args);
    va_end(args);

    return ret;
}

int __snprintf(char *a1, size_t a2, const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    int ret = vsnprintf(a1, a2, fmt, args); // TODO ehh
    va_end (args);

    return ret;
}

int __vsnprintf(char *a1, size_t a2, const char *fmt, va_list args)
{
    return vsnprintf(a1, a2, fmt, args); // TODO ehh
}

wchar_t* _wcscpy(wchar_t * dst, const wchar_t *src)
{
    if (!dst) return NULL;
    if (!src) return NULL;

    wchar_t *tmp = dst;
    while((*(dst++) = *(src++)));
    return tmp;
}

int jk_MessageBeep(int a)
{
    return 0;
}

int __strcmpi(const char *a, const char *b)
{
    int ca, cb;

    do 
    {
        ca = (unsigned char) *a++;
        cb = (unsigned char) *b++;
        ca = tolower(toupper(ca));
        cb = tolower(toupper(cb));
    }
    while (ca == cb && ca != '\0');

    return ca - cb;
}

int __strnicmp(const char *a, const char *b, size_t c)
{
    int ca, cb, n;

    n = 0;
    do 
    {
        if (n >= c) break;
        ca = (unsigned char) *a++;
        cb = (unsigned char) *b++;
        ca = tolower(toupper(ca));
        cb = tolower(toupper(cb));
        n++;
    }
    while (ca == cb && ca != '\0');

    return ca - cb;
}

char __tolower(char a)
{
    return tolower(a);
}

int msvc_sub_512D30(int a, int b)
{
    assert(0);
    return 0;
}

int jk_MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    assert(0);
    return 0;
}

int stdGdi_GetHwnd()
{
    //assert(0);
    return 0;
}

void jk_PostMessageA()
{
    assert(0);
}

void jk_GetCursorPos(LPPOINT lpPoint)
{
    assert(0);
}

int jk_GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase)
{
    return 0;
}

void jk_BeginPaint(int a, struct tagPAINTSTRUCT * lpPaint)
{
    assert(0);
}

int jk_vsnwprintf(wchar_t * a, size_t b, const wchar_t *fmt, va_list list)
{
    return vsnwprintf_(a, b, fmt, list);
}

void jk_EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint)
{
    assert(0);
}

HINSTANCE stdGdi_GetHInstance()
{
    assert(0);
    return 0;
}

int jk_LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName)
{
    assert(0);
    return 1;
}

void jk_SetCursor(HCURSOR hCursor)
{
    assert(0);
}

void jk_InvalidateRect(HWND hWnd, const RECT *lpRect, BOOL bErase)
{
    assert(0);
}

void jk_ChangeDisplaySettingsA(int a, int b)
{
    assert(0);
}

uint32_t jk_DirectDrawEnumerateA(void* a, void** b)
{
    assert(0);
    return 0;
}

uint32_t jk_DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter)
{
    assert(0);
    return 0;
}

uint32_t jk_DirectSoundCreate(LPGUID a, LPDIRECTSOUND *b, LPUNKNOWN c)
{
    assert(0);
    return 0;
}

uint32_t jk_DirectPlayLobbyCreateA(GUID *lpGUID, void** b, IUnknown* c, LPVOID d, int e)
{
    assert(0);
    return 0;
}

uint32_t jk_DirectInputCreateA(int a, int b, void** c, LPUNKNOWN d)
{
    assert(0);
    return 0;
}

uint32_t jk_CreateFileA()
{
    assert(0);
    return 0;
}

uint32_t jk_CreateFileMappingA()
{
    assert(0);
    return 0;
}

void* jk_LocalAlloc()
{
    assert(0);
    return 0;
}

uint32_t jk_MapViewOfFile()
{
    assert(0);
    return 0;
}

void jk_UnmapViewOfFile(LPCVOID lpBaseAddress)
{
    assert(0);
}

void jk_CloseHandle(HANDLE hObject)
{
    assert(0);
}

uint32_t jk_GetDesktopWindow()
{
    assert(0);
    return 0;
}

uint32_t jk_GetDC(HWND hWnd)
{
    assert(0);
    return 0;
}

uint32_t jk_GetDeviceCaps(HDC hdc, int index)
{
    assert(0);
    return 0;
}

uint32_t jk_WinExec(const char* a, int b)
{
    assert(0);
    return 0;
}

int _string_modify_idk(int c)
{
    return toupper(c);
}

void jk_ReleaseDC(HWND hWnd, HDC hDC)
{
    assert(0);
}

void jk_SetFocus(HWND hWnd)
{
    //assert(0);
}

void jk_SetActiveWindow(HWND hWnd)
{
    assert(0);
}

void jk_ShowCursor(int a)
{
    //assert(0);
    stdControl_ShowCursor(a);
}

void jk_ValidateRect(HWND hWnd, const RECT *lpRect)
{
    assert(0);
}

int __isspace(int a)
{
    return isspace(a & 0xFF);
}

int _iswspace(int a)
{
    unsigned char c = a & 0xFF;
    if (c == '\t' || c == '\n' ||
        c == '\v' || c == '\f' || c == '\r' || c == ' ') {
        return 1;
    }

    return isspace(c);
}

size_t __wcslen(const wchar_t * strarg)
{
    if(!strarg)
     return -1; //strarg is NULL pointer
   const wchar_t* str = strarg;
   for(;*str;++str)
     ; // empty body
   return str-strarg;
}

wchar_t* __wcscat(wchar_t * a, const wchar_t * b)
{
    wchar_t* ret = a;
    a += __wcslen(a);
    memmove(a, b, __wcslen(b) * sizeof(wchar_t));
    return ret;
}

wchar_t* __wcschr(const wchar_t * s, wchar_t c)
{
    do {
        if (*s == c)
        {
        return (wchar_t*)s;
        }
    } while (*s++);
    return NULL;
}

wchar_t* __wcsncpy(wchar_t * a, const wchar_t * b, size_t c)
{
    wchar_t* ret = a;
    size_t len = __wcslen(b) * sizeof(wchar_t);
    if (len > c*sizeof(wchar_t)) {
        len = c*sizeof(wchar_t);
    }
    memmove(a, b, len);
    a[len] = 0;
    return &a[len];
}

wchar_t* __wcsrchr(const wchar_t * s, wchar_t c)
{
    wchar_t *rtnval = 0;
    do {
        if (*s == c)
            rtnval = (wchar_t*) s;
        } while (*s++);
    return (rtnval);
}

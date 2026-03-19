/*
 * PLATFORM.H  --  OS detection and platform abstraction entry point
 *
 * All OS-specific constants, types, and prototypes are centralised here.
 * Code in other modules must NOT include <dirent.h>, <termios.h>, etc.
 * directly; include this header instead.
 *
 * Supported platforms:
 *   PLATFORM_X68K   : X68000 / Human68k (m68k-xelf-gcc toolchain)
 *   PLATFORM_POSIX  : Ubuntu / Linux / macOS and other POSIX systems
 *
 * Note: The m68k-xelf toolchain for X68K provides POSIX-compatible headers
 * (dirent.h, termios.h, sys/stat.h) so most I/O code is shared.
 * Only action_open() differs between platforms (xdg-open vs _dos_exec2).
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/* ------------------------------------------------------------------ */
/* Platform detection                                                   */
/* ------------------------------------------------------------------ */

#if defined(__human68k__) || defined(__HUMAN68K__) || \
    defined(__m68k__)     || defined(__mc68000__)
#  define PLATFORM_X68K  1
#else
#  define PLATFORM_POSIX 1
#endif

/* ------------------------------------------------------------------ */
/* Common includes (available on both platforms)                        */
/* ------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* POSIX includes (available on both POSIX and X68K toolchain)         */
/* NOTE: termios.h is NOT included here; X68K toolchain lacks          */
/* sys/termios.h.  UITUI.C includes termios.h conditionally.          */
/* ------------------------------------------------------------------ */

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* Path constants                                                       */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_X68K
#  define PATH_SEP       '\\'
#  define PATH_SEP_STR   "\\"
#  define PATH_MAX_LEN   96       /* Human68k practical limit */
#else
#  define PATH_SEP       '/'
#  define PATH_SEP_STR   "/"
#  ifndef PATH_MAX_LEN
#    define PATH_MAX_LEN 1024
#  endif
#endif

/* ------------------------------------------------------------------ */
/* Filesystem iteration  (POSIX DIR on both platforms)                  */
/* ------------------------------------------------------------------ */

typedef struct {
    DIR  *dp;
    char  dirpath[PATH_MAX_LEN];
} DirIter;

/* ------------------------------------------------------------------ */
/* Entry type returned by diriter_next()                               */
/* ------------------------------------------------------------------ */

#define ENTRY_FILE 'F'
#define ENTRY_DIR  'D'
#define ENTRY_NONE  0

typedef struct {
    char type;               /* ENTRY_FILE or ENTRY_DIR */
    char name[PATH_MAX_LEN]; /* base name only           */
} DirEntry;

/* ------------------------------------------------------------------ */
/* Portable basename helper                                             */
/* ------------------------------------------------------------------ */

static inline const char *plat_basename(const char *path)
{
    const char *p    = path;
    const char *last = path;
    while (*p) {
        if (*p == '/' || *p == '\\')
            last = p + 1;
        p++;
    }
    return last;
}

/* ------------------------------------------------------------------ */
/* Portable path-join helper                                            */
/* ------------------------------------------------------------------ */

static inline void plat_join(char *dst, size_t dstsz,
                              const char *dir, const char *name)
{
    size_t dlen = strlen(dir);
    if (dlen > 0 && dir[dlen-1] != '/' && dir[dlen-1] != '\\') {
        snprintf(dst, dstsz, "%s%c%s", dir, PATH_SEP, name);
    } else {
        snprintf(dst, dstsz, "%s%s", dir, name);
    }
}

/* ------------------------------------------------------------------ */
/* Case-insensitive string functions                                    */
/* ------------------------------------------------------------------ */

static inline int plat_strcasecmp(const char *a, const char *b)
{
    for (;;) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        if (!ca)      return 0;
        a++; b++;
    }
}

static inline const char *plat_strcasestr(const char *haystack,
                                           const char *needle)
{
    size_t nlen = strlen(needle);
    if (nlen == 0) return haystack;
    for (; *haystack; haystack++) {
        if (tolower((unsigned char)*haystack) ==
            tolower((unsigned char)*needle)) {
            size_t i;
            for (i = 1; i < nlen; i++) {
                if (tolower((unsigned char)haystack[i]) !=
                    tolower((unsigned char)needle[i]))
                    break;
            }
            if (i == nlen) return haystack;
        }
    }
    return NULL;
}

#endif /* PLATFORM_H */

/*
 * ACTIONS.C  --  open / cd implementation
 *
 * Filesystem operations (stat, is_dir) use POSIX on both platforms.
 * The m68k-xelf toolchain for X68K provides POSIX stat/unistd support.
 *
 * Only action_open() differs:
 *   Ubuntu : xdg-open for files; execv for executables
 *   X68K   : _dos_exec2 for executables; _dos_exec2 for everything
 */

#include "ACTIONS.H"
#include "PLATFORM.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* Common helpers  (POSIX stat on both platforms)                      */
/* ------------------------------------------------------------------ */

static int is_dir(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static void parent_dir(const char *path, char *out, size_t outsz)
{
    const char *base = plat_basename(path);
    size_t dirlen;

    if (base == path) {
        strncpy(out, ".", outsz - 1);
        out[outsz - 1] = '\0';
        return;
    }
    dirlen = (size_t)(base - path);
    if (dirlen >= outsz) dirlen = outsz - 1;
    memcpy(out, path, dirlen);
    while (dirlen > 1 &&
           (out[dirlen-1] == '/' || out[dirlen-1] == '\\'))
        dirlen--;
    out[dirlen] = '\0';
}

/* ------------------------------------------------------------------ */
/* action_open  (platform-specific)                                    */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_X68K

#include <x68k/dos.h>

int action_open(const char *path)
{
    if (is_dir(path)) return action_cd(path);

    /* On Human68k, launch executable via DOS EXEC2 */
    return _dos_exec2(0, path, "", "");
}

#else /* PLATFORM_POSIX */

static int is_executable(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 &&
            S_ISREG(st.st_mode) &&
            (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
}

int action_open(const char *path)
{
    if (is_dir(path)) return action_cd(path);

    if (is_executable(path)) {
        char cmd[PATH_MAX_LEN + 4];
        snprintf(cmd, sizeof(cmd), "\"%s\"", path);
        return system(cmd);
    }

    {
        char cmd[PATH_MAX_LEN + 16];
        snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" &", path);
        return system(cmd);
    }
}

#endif /* PLATFORM_X68K / PLATFORM_POSIX */

/* ------------------------------------------------------------------ */
/* action_cd  (platform-independent)                                   */
/* ------------------------------------------------------------------ */

int action_cd(const char *path)
{
    char target[PATH_MAX_LEN];
    FILE *fp;

    if (is_dir(path)) {
        strncpy(target, path, PATH_MAX_LEN - 1);
        target[PATH_MAX_LEN - 1] = '\0';
    } else {
        parent_dir(path, target, sizeof(target));
    }

    fp = fopen(XFIND_CD_FILE, "w");
    if (!fp) {
        fprintf(stderr, "xfind: cannot write cd file: %s\n", XFIND_CD_FILE);
        return -1;
    }
    fprintf(fp, "%s\n", target);
    fclose(fp);

    printf("cd: %s\n", target);
    return 0;
}

/*
 * ACTIONS.C  --  open / cd implementation
 */

#include "ACTIONS.H"
#include "PLATFORM.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static void parent_dir(const char *path, char *out, size_t outsz)
{
    const char *base = plat_basename(path);
    size_t dirlen;

    if (base == path) {
        /* no separator found: current directory */
        strncpy(out, ".", outsz - 1);
        out[outsz - 1] = '\0';
        return;
    }
    dirlen = (size_t)(base - path);
    if (dirlen >= outsz) dirlen = outsz - 1;
    memcpy(out, path, dirlen);
    /* strip trailing separator */
    while (dirlen > 1 &&
           (out[dirlen-1] == '/' || out[dirlen-1] == '\\'))
        dirlen--;
    out[dirlen] = '\0';
}

/* ------------------------------------------------------------------ */
/* POSIX implementation                                                 */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_POSIX

#include <sys/stat.h>
#include <unistd.h>

static int is_dir(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static int is_executable(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 &&
            S_ISREG(st.st_mode) &&
            (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
}

int action_open(const char *path)
{
    if (is_dir(path))        return action_cd(path);
    if (is_executable(path)) {
        /* Launch executable directly */
        char cmd[PATH_MAX_LEN + 4];
        snprintf(cmd, sizeof(cmd), "\"%s\"", path);
        return system(cmd);
    }
    /* Use xdg-open for everything else */
    {
        char cmd[PATH_MAX_LEN + 16];
        snprintf(cmd, sizeof(cmd), "xdg-open \"%s\"", path);
        return system(cmd);
    }
}

#endif /* PLATFORM_POSIX */

/* ------------------------------------------------------------------ */
/* Human68k implementation                                              */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_X68K

#include <sys/dos.h>

static int is_dir(const char *path)
{
    struct _finddata_t fd;
    return (_dos_findfirst(path, 0x17, &fd) == 0 && (fd.attrib & 0x10));
}

int action_open(const char *path)
{
    if (is_dir(path)) return action_cd(path);
    /* On Human68k just execute via EXEC */
    return _dos_exec(path, NULL, NULL);
}

#endif /* PLATFORM_X68K */

/* ------------------------------------------------------------------ */
/* action_cd  (platform-independent)                                   */
/* ------------------------------------------------------------------ */

int action_cd(const char *path)
{
    char target[PATH_MAX_LEN];
    FILE *fp;

    /* Determine target directory */
#ifdef PLATFORM_POSIX
    {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            strncpy(target, path, PATH_MAX_LEN - 1);
            target[PATH_MAX_LEN - 1] = '\0';
        } else {
            parent_dir(path, target, sizeof(target));
        }
    }
#else
    {
        /* X68K: assume path type from listing */
        struct _finddata_t fd;
        if (_dos_findfirst(path, 0x17, &fd) == 0 && (fd.attrib & 0x10)) {
            strncpy(target, path, PATH_MAX_LEN - 1);
            target[PATH_MAX_LEN - 1] = '\0';
        } else {
            parent_dir(path, target, sizeof(target));
        }
    }
#endif

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

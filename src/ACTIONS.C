/*
 * ACTIONS.C  --  open / cd implementation
 *
 * Filesystem operations (stat, is_dir) use POSIX on both platforms.
 * The m68k-xelf toolchain for X68K provides POSIX stat/unistd support.
 *
 * action_open() behaviour by platform:
 *   POSIX : executable -> direct exec; other -> opener chain; dir -> cd
 *           Opener chain: -O/open_cmd > XFIND_OPEN env > xdg-open/open/mimeopen/less > print path
 *   X68K  : executable (.X/.COM/.BAT) -> _dos_exec2; other -> opener chain; dir -> cd
 *           Opener chain: -O/open_cmd > XFIND_OPEN env > TYPE (default built-in)
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
/* POSIX: opener resolution chain                                       */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_POSIX

static int is_executable(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 &&
            S_ISREG(st.st_mode) &&
            (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
}

/*
 * Return cmd if it is found as an executable in PATH, NULL otherwise.
 * Handles absolute paths directly; bare names are searched in PATH.
 */
static int cmd_in_path(const char *cmd)
{
    const char *path_env;
    const char *p, *q;
    char buf[PATH_MAX_LEN];

    if (!cmd || !*cmd) return 0;

    /* Absolute or relative path: check directly */
    if (strchr(cmd, '/') || strchr(cmd, '\\'))
        return access(cmd, X_OK) == 0;

    path_env = getenv("PATH");
    if (!path_env) return 0;

    p = path_env;
    while (*p) {
        q = strchr(p, ':');
        {
            size_t dlen = q ? (size_t)(q - p) : strlen(p);
            if (dlen + 1 + strlen(cmd) + 1 < sizeof(buf)) {
                memcpy(buf, p, dlen);
                buf[dlen] = '/';
                strcpy(buf + dlen + 1, cmd);
                if (access(buf, X_OK) == 0) return 1;
            }
        }
        if (!q) break;
        p = q + 1;
    }
    return 0;
}

/*
 * Resolve the opener to use for non-executable files.
 * Priority:
 *   1. override   (from -O / open_cmd config -- already merged by caller)
 *   2. XFIND_OPEN env var
 *   3. auto-detect: xdg-open, open, mimeopen, less
 *   4. NULL  => fallback: print path
 */
static const char *resolve_opener(const char *override)
{
    static const char *candidates[] = {"xdg-open", "open", "mimeopen", "less", NULL};
    const char *env;
    int i;

    if (override && *override) return override;

    env = getenv("XFIND_OPEN");
    if (env && *env) return env;

    for (i = 0; candidates[i]; i++) {
        if (cmd_in_path(candidates[i])) return candidates[i];
    }
    return NULL; /* fallback */
}

int action_open(const char *path, const char *opener)
{
    const char *o;
    char cmd[PATH_MAX_LEN + 64];

    if (is_dir(path)) return action_cd(path);

    if (is_executable(path)) {
        snprintf(cmd, sizeof(cmd), "\"%s\"", path);
        return system(cmd);
    }

    o = resolve_opener(opener);
    if (o) {
        snprintf(cmd, sizeof(cmd), "%s \"%s\"", o, path);
        return system(cmd);
    }

    /* Fallback: print path so the user can act on it */
    printf("%s\n", path);
    return 0;
}

#endif /* PLATFORM_POSIX */

/* ------------------------------------------------------------------ */
/* X68K: executable files via _dos_exec2; others via opener chain     */
/* Opener chain: opener arg > XFIND_OPEN env > TYPE (default)         */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_X68K

#include <x68k/dos.h>

/*
 * On Human68k, executable files have .X, .COM, or .BAT extensions.
 */
static int is_x68k_executable(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext) return 0;
    return (plat_strcasecmp(ext, ".x")   == 0 ||
            plat_strcasecmp(ext, ".com") == 0 ||
            plat_strcasecmp(ext, ".bat") == 0);
}

int action_open(const char *path, const char *opener)
{
    const char *o;
    char cmd[PATH_MAX_LEN + 16];

    if (is_dir(path)) return action_cd(path);

    /* Executable files: run directly */
    if (is_x68k_executable(path)) {
        return _dos_exec2(0, path, "", "");
    }

    /* Non-executable files: opener chain */
    if (opener && *opener) {
        o = opener;
    } else {
        o = getenv("XFIND_OPEN");
        if (!o || !*o) o = "TYPE"; /* Human68k shell built-in */
    }

    snprintf(cmd, sizeof(cmd), "%s %s", o, path);
    return system(cmd);
}

#endif /* PLATFORM_X68K */

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

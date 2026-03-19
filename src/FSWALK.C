/*
 * FSWALK.C  --  Recursive filesystem walker implementation
 */

#include "FSWALK.H"

/* ------------------------------------------------------------------ */
/* POSIX implementation                                                 */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_POSIX

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

static int walk_dir(const char *path, FswalkCallback cb, void *ud)
{
    DIR *dp;
    struct dirent *ent;
    char child[PATH_MAX_LEN];
    struct stat st;

    dp = opendir(path);
    if (!dp) return -1;

    /* Emit the directory itself first */
    cb(ENTRY_DIR, path, ud);

    while ((ent = readdir(dp)) != NULL) {
        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' ||
             (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;

        plat_join(child, sizeof(child), path, ent->d_name);

        if (stat(child, &st) != 0)
            continue; /* skip unreadable */

        if (S_ISDIR(st.st_mode)) {
            walk_dir(child, cb, ud); /* recurse */
        } else if (S_ISREG(st.st_mode)) {
            cb(ENTRY_FILE, child, ud);
        }
        /* symlinks and specials: skip */
    }

    closedir(dp);
    return 0;
}

int fswalk_run(const char *root, FswalkCallback cb, void *userdata)
{
    struct stat st;
    if (stat(root, &st) != 0) return -1;
    if (!S_ISDIR(st.st_mode)) {
        /* root is a regular file: emit it directly */
        cb(ENTRY_FILE, root, userdata);
        return 0;
    }
    return walk_dir(root, cb, userdata);
}

#endif /* PLATFORM_POSIX */

/* ------------------------------------------------------------------ */
/* Human68k implementation                                              */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_X68K

#include <sys/dos.h>
#include <string.h>
#include <stdio.h>

static int walk_dir(const char *path, FswalkCallback cb, void *ud)
{
    struct _finddata_t fd;
    char pat[PATH_MAX_LEN];
    char child[PATH_MAX_LEN];
    int h;

    /* Emit directory first */
    cb(ENTRY_DIR, path, ud);

    /* Build wildcard pattern */
    plat_join(pat, sizeof(pat), path, "*.*");

    h = _dos_findfirst(pat, 0x17, &fd);
    while (h == 0) {
        if (!(fd.attrib & 0x10) || /* file */
             (fd.attrib & 0x10)) { /* dir  */
            if (fd.name[0] == '.' &&
                (fd.name[1] == '\0' ||
                 (fd.name[1] == '.' && fd.name[2] == '\0'))) {
                h = _dos_findnext(&fd);
                continue;
            }
            plat_join(child, sizeof(child), path, fd.name);
            if (fd.attrib & 0x10) {
                walk_dir(child, cb, ud);
            } else {
                cb(ENTRY_FILE, child, ud);
            }
        }
        h = _dos_findnext(&fd);
    }
    _dos_findclose(&fd);
    return 0;
}

int fswalk_run(const char *root, FswalkCallback cb, void *userdata)
{
    return walk_dir(root, cb, userdata);
}

#endif /* PLATFORM_X68K */

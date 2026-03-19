/*
 * FSWALK.C  --  Recursive filesystem walker implementation
 *
 * Uses POSIX opendir/readdir/stat on both Ubuntu and X68K
 * (the m68k-xelf toolchain provides full POSIX dirent/stat support).
 */

#include "FSWALK.H"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

static int walk_dir(const char *path, FswalkCallback cb, void *ud)
{
    DIR            *dp;
    struct dirent  *ent;
    char            child[PATH_MAX_LEN];
    struct stat     st;

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
            continue; /* skip unreadable / broken symlinks */

        if (S_ISDIR(st.st_mode)) {
            walk_dir(child, cb, ud); /* recurse */
        } else if (S_ISREG(st.st_mode)) {
            cb(ENTRY_FILE, child, ud);
        }
        /* symlinks to non-regular, devices, etc.: skip */
    }

    closedir(dp);
    return 0;
}

int fswalk_run(const char *root, FswalkCallback cb, void *userdata)
{
    struct stat st;
    if (stat(root, &st) != 0) return -1;
    if (!S_ISDIR(st.st_mode)) {
        cb(ENTRY_FILE, root, userdata);
        return 0;
    }
    return walk_dir(root, cb, userdata);
}

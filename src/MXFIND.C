/*
 * MXFIND.C  --  xfind entry point
 *
 * Usage:
 *   xfind [options] [QUERY]
 *
 * Options:
 *   -i FILE   index file (default: xfind.idx in cwd, then $HOME/.xfind.idx)
 *   -c FILE   config file
 *   --open    non-interactive: open best match
 *   --cd      non-interactive: cd to best match
 */

#include "PLATFORM.H"
#include "INDEX.H"
#include "MATCH.H"
#include "ACTIONS.H"
#include "CONFIG.H"
#include "UITUI.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Locate the index file                                                */
/* ------------------------------------------------------------------ */

/* POSIX stat is available on both Ubuntu and X68K (m68k-xelf toolchain) */
#include <sys/stat.h>
static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static const char *find_index(const char *override)
{
    static char buf[PATH_MAX_LEN];

    if (override) return override;

    /* 1. Current directory */
    if (file_exists(INDEX_DEFAULT_NAME)) return INDEX_DEFAULT_NAME;

    /* 2. $HOME/.xfind.idx */
    {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(buf, sizeof(buf), "%s%c.xfind.idx", home, PATH_SEP);
            if (file_exists(buf)) return buf;
        }
    }

    /* 3. Fallback to cwd name (will produce a load error with a clear msg) */
    return INDEX_DEFAULT_NAME;
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

static void usage(void)
{
    fprintf(stderr,
        "Usage: xfind [-i INDEXFILE] [-c CONFIG] [--open|--cd] [QUERY]\n");
}

int main(int argc, char *argv[])
{
    const char *idxfile  = NULL;
    const char *cfgfile  = NULL;
    int         mode_open = 0; /* non-interactive open */
    int         mode_cd   = 0; /* non-interactive cd   */
    const char *query    = NULL;
    int         i;
    Config      cfg;
    IndexTable  table;
    MatchSet    set;
    int         ret = 0;

    cfg.head = NULL;

    /* Parse options */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            idxfile = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cfgfile = argv[++i];
        } else if (strcmp(argv[i], "--open") == 0) {
            mode_open = 1;
        } else if (strcmp(argv[i], "--cd") == 0) {
            mode_cd = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "xfind: unknown option: %s\n", argv[i]);
            usage();
            return 1;
        } else {
            query = argv[i];
            i++;
            break;
        }
    }

    /* Load config */
    if (cfgfile) cfg_load(&cfg, cfgfile);

    /* Determine index path */
    idxfile = find_index(idxfile);

    /* Load index */
    if (idx_load(&table, idxfile) != 0) {
        fprintf(stderr, "xfind: cannot load index: %s\n", idxfile);
        fprintf(stderr, "  Run 'xfidx <root>' first to build an index.\n");
        cfg_free(&cfg);
        return 1;
    }

    /* Search */
    if (match_search(&table, query ? query : "", &set) != 0) {
        fprintf(stderr, "xfind: search error\n");
        idx_table_free(&table);
        cfg_free(&cfg);
        return 1;
    }

    if (set.count == 0) {
        printf("xfind: no match for '%s'\n", query ? query : "");
        ret = 1;
        goto cleanup;
    }

    /* Non-interactive modes */
    if (mode_open) {
        ret = action_open(set.results[0].entry->path);
        goto cleanup;
    }
    if (mode_cd) {
        ret = action_cd(set.results[0].entry->path);
        goto cleanup;
    }

    /* Interactive TUI */
    ret = tui_run(&set);

cleanup:
    match_free(&set);
    idx_table_free(&table);
    cfg_free(&cfg);
    return ret;
}

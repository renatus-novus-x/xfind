/*
 * MXFIDX.C  --  xfidx entry point
 *
 * Usage:
 *   xfidx [options] ROOT [ROOT ...]
 *
 * Options:
 *   -o FILE   output index file (default: xfind.idx in cwd)
 *   -c FILE   config file
 *   -q        quiet (suppress progress)
 */

#include "PLATFORM.H"
#include "FSWALK.H"
#include "INDEX.H"
#include "CONFIG.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Walk callback state                                                  */
/* ------------------------------------------------------------------ */

typedef struct {
    IndexWriter *writer;
    int          quiet;
    long         count;
} WalkState;

static int on_entry(char type, const char *fullpath, void *ud)
{
    WalkState *s = (WalkState *)ud;
    idx_writer_entry(s->writer, type, fullpath);
    s->count++;
    if (!s->quiet && (s->count % 1000 == 0)) {
        fprintf(stderr, "\r  %ld entries...", s->count);
        fflush(stderr);
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

static void usage(void)
{
    fprintf(stderr,
        "Usage: xfidx [-o OUTFILE] [-c CONFIG] [-q] ROOT [ROOT ...]\n");
}

int main(int argc, char *argv[])
{
    const char *outfile  = INDEX_DEFAULT_NAME;
    const char *cfgfile  = NULL;
    int         quiet    = 0;
    int         i;
    int         roots    = 0;
    Config      cfg;
    IndexWriter writer;
    WalkState   state;

    cfg.head = NULL;

    /* Parse options */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cfgfile = argv[++i];
        } else if (strcmp(argv[i], "-q") == 0) {
            quiet = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "xfidx: unknown option: %s\n", argv[i]);
            usage();
            return 1;
        } else {
            break; /* first non-option = first ROOT */
        }
    }

    if (i >= argc) {
        fprintf(stderr, "xfidx: no ROOT specified\n");
        usage();
        return 1;
    }

    /* Load config (not fatal if missing) */
    if (cfgfile) cfg_load(&cfg, cfgfile);

    /* Open writer */
    if (idx_writer_open(&writer, outfile) != 0) {
        fprintf(stderr, "xfidx: cannot create output file: %s\n", outfile);
        cfg_free(&cfg);
        return 1;
    }

    /* Write ROOT headers */
    for (roots = i; roots < argc; roots++) {
        idx_writer_root(&writer, argv[roots]);
    }

    /* Walk each root */
    state.writer = &writer;
    state.quiet  = quiet;
    state.count  = 0;

    for (; i < argc; i++) {
        if (!quiet) fprintf(stderr, "Scanning: %s\n", argv[i]);
        if (fswalk_run(argv[i], on_entry, &state) != 0) {
            fprintf(stderr, "xfidx: cannot open: %s\n", argv[i]);
        }
    }

    idx_writer_close(&writer);

    if (!quiet) {
        fprintf(stderr, "\r  %ld entries written to %s\n",
                state.count, outfile);
    }

    cfg_free(&cfg);
    return 0;
}

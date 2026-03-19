/*
 * INDEX.H  --  Text-format index I/O
 *
 * Index file format:
 *
 *   XFIND-INDEX 1
 *   ROOT /some/path
 *   ENTRY F /some/path/file.txt
 *   ENTRY D /some/path/subdir
 */

#ifndef INDEX_H
#define INDEX_H

#include "PLATFORM.H"

#define INDEX_VERSION  "1"
#define INDEX_MAGIC    "XFIND-INDEX"

/* Maximum number of entries held in memory */
#define INDEX_MAX_ENTRIES 65536

typedef struct {
    char type;               /* ENTRY_FILE ('F') or ENTRY_DIR ('D') */
    char path[PATH_MAX_LEN]; /* full, normalised path                */
} IndexEntry;

typedef struct {
    IndexEntry *entries;     /* heap-allocated array                 */
    int         count;
    int         capacity;
} IndexTable;

/* ------------------------------------------------------------------ */
/* Writer API  (used by xfidx)                                         */
/* ------------------------------------------------------------------ */

typedef struct {
    FILE *fp;
} IndexWriter;

/* Open/close the output file; returns 0 on success */
int  idx_writer_open(IndexWriter *w, const char *path);
void idx_writer_root(IndexWriter *w, const char *root);
void idx_writer_entry(IndexWriter *w, char type, const char *fullpath);
void idx_writer_close(IndexWriter *w);

/* ------------------------------------------------------------------ */
/* Reader API  (used by xfind)                                         */
/* ------------------------------------------------------------------ */

/*
 * Load index from file into table.
 * table->entries is heap-allocated; caller must call idx_table_free().
 * Returns 0 on success, -1 on error.
 */
int  idx_load(IndexTable *table, const char *path);
void idx_table_free(IndexTable *table);

/* Default index file name */
#define INDEX_DEFAULT_NAME "xfind.idx"

#endif /* INDEX_H */

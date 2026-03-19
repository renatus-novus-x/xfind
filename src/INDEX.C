/*
 * INDEX.C  --  Text-format index I/O implementation
 */

#include "INDEX.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Writer                                                               */
/* ------------------------------------------------------------------ */

int idx_writer_open(IndexWriter *w, const char *path)
{
    w->fp = fopen(path, "w");
    if (!w->fp) return -1;
    fprintf(w->fp, "%s %s\n", INDEX_MAGIC, INDEX_VERSION);
    return 0;
}

void idx_writer_root(IndexWriter *w, const char *root)
{
    if (!w->fp) return;
    fprintf(w->fp, "ROOT %s\n", root);
}

void idx_writer_entry(IndexWriter *w, char type, const char *fullpath)
{
    if (!w->fp) return;
    fprintf(w->fp, "ENTRY %c %s\n", type, fullpath);
}

void idx_writer_close(IndexWriter *w)
{
    if (w->fp) {
        fclose(w->fp);
        w->fp = NULL;
    }
}

/* ------------------------------------------------------------------ */
/* Reader                                                               */
/* ------------------------------------------------------------------ */

static int table_push(IndexTable *t, char type, const char *path)
{
    IndexEntry *e;
    if (t->count >= INDEX_MAX_ENTRIES) return -1;
    if (t->count >= t->capacity) {
        int newcap = t->capacity ? t->capacity * 2 : 256;
        IndexEntry *newbuf = (IndexEntry *)realloc(t->entries,
                                newcap * sizeof(IndexEntry));
        if (!newbuf) return -1;
        t->entries  = newbuf;
        t->capacity = newcap;
    }
    e = &t->entries[t->count++];
    e->type = type;
    strncpy(e->path, path, PATH_MAX_LEN - 1);
    e->path[PATH_MAX_LEN - 1] = '\0';
    return 0;
}

int idx_load(IndexTable *table, const char *path)
{
    FILE *fp;
    char  line[PATH_MAX_LEN + 16];
    int   header_seen = 0;

    table->entries  = NULL;
    table->count    = 0;
    table->capacity = 0;

    fp = fopen(path, "r");
    if (!fp) return -1;

    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        /* strip trailing newline */
        if (len > 0 && line[len-1] == '\n') line[--len] = '\0';
        if (len > 0 && line[len-1] == '\r') line[--len] = '\0';

        if (!header_seen) {
            if (strncmp(line, INDEX_MAGIC, strlen(INDEX_MAGIC)) == 0) {
                header_seen = 1;
            }
            continue;
        }

        if (strncmp(line, "ENTRY ", 6) == 0 && len > 8) {
            char type    = line[6];
            const char *p = line + 8; /* skip "ENTRY X " */
            if (type == ENTRY_FILE || type == ENTRY_DIR) {
                table_push(table, type, p);
            }
        }
        /* ROOT lines are informational; skip */
    }

    fclose(fp);
    return 0;
}

void idx_table_free(IndexTable *table)
{
    free(table->entries);
    table->entries  = NULL;
    table->count    = 0;
    table->capacity = 0;
}

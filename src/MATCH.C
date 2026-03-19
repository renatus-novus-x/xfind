/*
 * MATCH.C  --  Search and ranking implementation
 */

#include "MATCH.H"
#include "PLATFORM.H"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Compute rank for one entry                                           */
/* ------------------------------------------------------------------ */

static int compute_rank(const IndexEntry *e, const char *query)
{
    const char *base = plat_basename(e->path);

    if (plat_strcasecmp(base, query) == 0)          return 0;
    if (plat_strcasestr(base, query) == base)        return 1;
    if (plat_strcasestr(base, query) != NULL)        return 2;
    if (plat_strcasestr(e->path, query) != NULL)     return 3;
    return -1; /* no match */
}

/* ------------------------------------------------------------------ */
/* Comparison function for qsort                                        */
/* ------------------------------------------------------------------ */

static int cmp_results(const void *a, const void *b)
{
    const MatchResult *ma = (const MatchResult *)a;
    const MatchResult *mb = (const MatchResult *)b;
    int d;

    /* Primary: rank */
    d = ma->rank - mb->rank;
    if (d) return d;

    /* Secondary: shorter basename */
    d = (int)strlen(plat_basename(ma->entry->path))
      - (int)strlen(plat_basename(mb->entry->path));
    if (d) return d;

    /* Tertiary: shorter fullpath */
    d = (int)strlen(ma->entry->path)
      - (int)strlen(mb->entry->path);
    if (d) return d;

    /* Quaternary: lexicographic fullpath */
    d = strcmp(ma->entry->path, mb->entry->path);
    if (d) return d;

    /* Final tiebreaker: original order (stable sort) */
    return ma->orig - mb->orig;
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

int match_search(const IndexTable *table, const char *query, MatchSet *set)
{
    int i;
    int cap = table->count ? table->count : 1;
    MatchResult *buf = (MatchResult *)malloc(cap * sizeof(MatchResult));
    if (!buf) return -1;

    set->results = buf;
    set->count   = 0;

    if (!query || query[0] == '\0') {
        /* Empty query: return everything in original order */
        for (i = 0; i < table->count; i++) {
            buf[set->count].entry = &table->entries[i];
            buf[set->count].rank  = 4; /* below ranked results */
            buf[set->count].orig  = i;
            set->count++;
        }
    } else {
        for (i = 0; i < table->count; i++) {
            int r = compute_rank(&table->entries[i], query);
            if (r >= 0) {
                buf[set->count].entry = &table->entries[i];
                buf[set->count].rank  = r;
                buf[set->count].orig  = i;
                set->count++;
            }
        }
    }

    if (set->count > 1) {
        qsort(buf, (size_t)set->count, sizeof(MatchResult), cmp_results);
    }

    return 0;
}

void match_free(MatchSet *set)
{
    free(set->results);
    set->results = NULL;
    set->count   = 0;
}

/*
 * MATCH.H  --  Search and ranking
 *
 * Ranking priority (lower score = better match):
 *   0  basename exact match
 *   1  basename prefix match
 *   2  basename substring match
 *   3  fullpath substring match
 *
 * Secondary: shorter basename, shorter fullpath, then lexicographic.
 * Sort is stable (ties broken by original index order).
 */

#ifndef MATCH_H
#define MATCH_H

#include "INDEX.H"

#define MATCH_MAX_RESULTS INDEX_MAX_ENTRIES

typedef struct {
    const IndexEntry *entry;   /* pointer into the loaded IndexTable  */
    int               rank;    /* 0=best … 3=worst                    */
    int               orig;    /* original position for stable sort   */
} MatchResult;

typedef struct {
    MatchResult *results;  /* heap-allocated */
    int          count;
} MatchSet;

/*
 * Search table for query (case-insensitive).
 * Populates set (caller must call match_free() afterwards).
 * Returns 0 on success.
 */
int  match_search(const IndexTable *table, const char *query, MatchSet *set);
void match_free(MatchSet *set);

#endif /* MATCH_H */

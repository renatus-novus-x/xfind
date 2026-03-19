/*
 * FSWALK.H  --  Recursive filesystem walker
 */

#ifndef FSWALK_H
#define FSWALK_H

#include "PLATFORM.H"

/*
 * Callback invoked for every entry found during a walk.
 *
 *   type     : ENTRY_FILE or ENTRY_DIR
 *   fullpath : absolute, normalised path
 *   userdata : pointer passed through from fswalk_run()
 *
 * Return 0 to continue, non-zero to abort the entire walk.
 */
typedef int (*FswalkCallback)(char type, const char *fullpath, void *userdata);

/*
 * Walk root recursively, calling cb for every file and directory found.
 * Errors on individual entries are skipped; the walk continues.
 *
 * Returns 0 on success, -1 if root itself cannot be opened.
 */
int fswalk_run(const char *root, FswalkCallback cb, void *userdata);

#endif /* FSWALK_H */

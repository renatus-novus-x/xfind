/*
 * ACTIONS.H  --  open / cd actions
 *
 * cd cannot change the parent shell's directory directly.
 * Instead it writes the target path to XFIND_CD_FILE, which a shell
 * wrapper function can read and execute "cd" with.
 *
 * Recommended shell setup (bash/zsh):
 *   xcd() { xfind "$@"; p=$(cat "$TMPDIR/xfind_cd" 2>/dev/null); \
 *            [ -n "$p" ] && cd "$p" && rm -f "$TMPDIR/xfind_cd"; }
 */

#ifndef ACTIONS_H
#define ACTIONS_H

/* Path written by action_cd() for the shell wrapper to read */
#define XFIND_CD_FILE  "/tmp/xfind_cd"

/*
 * Open a file or directory with the platform's default mechanism.
 *   - executable file : execv / EXEC2
 *   - other file      : xdg-open (Ubuntu) / built-in opener (X68K)
 *   - directory       : treated as cd (calls action_cd internally)
 * Returns 0 on success, -1 on error.
 */
int action_open(const char *path);

/*
 * Write the cd target to XFIND_CD_FILE and print a hint.
 * If path is a file, writes its parent directory.
 * Returns 0 on success, -1 on error.
 */
int action_cd(const char *path);

#endif /* ACTIONS_H */

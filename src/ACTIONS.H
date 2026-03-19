/*
 * ACTIONS.H  --  open / cd actions
 *
 * cd cannot change the parent shell's directory directly.
 * Instead it writes the target path to XFIND_CD_FILE, which a shell
 * wrapper function can read and execute "cd" with.
 *
 * Recommended shell setup (bash/zsh):
 *   xcd() { xfind "$@"; p=$(cat /tmp/xfind_cd 2>/dev/null); \
 *            [ -n "$p" ] && cd "$p" && rm -f /tmp/xfind_cd; }
 *
 * Opener resolution order (non-executable files):
 *   POSIX:
 *     1. opener argument passed to action_open() -- from -O / open_cmd config
 *     2. XFIND_OPEN environment variable
 *     3. auto-detect: xdg-open, open, mimeopen, less  (first found in PATH)
 *     4. fallback: print path to stdout, exit 0
 *   X68K:
 *     1. opener argument passed to action_open() -- from -O / open_cmd config
 *     2. XFIND_OPEN environment variable
 *     3. TYPE  (Human68k shell built-in, always available)
 */

#ifndef ACTIONS_H
#define ACTIONS_H

/* Path written by action_cd() for the shell wrapper to read */
#define XFIND_CD_FILE  "/tmp/xfind_cd"

/*
 * Open a file or directory.
 *
 *   path   : target file or directory
 *   opener : opener command override (NULL = use env / auto-detect / default)
 *            POSIX: NULL triggers env/auto-detect/fallback chain.
 *            X68K:  NULL triggers env/TYPE chain.
 *
 * Behaviour:
 *   - directory       : calls action_cd()
 *   - executable file : runs directly (POSIX) / _dos_exec2 (X68K)
 *   - other file      : resolves opener per the chain above
 *
 * Returns 0 on success (including fallback path-print), -1 on hard error.
 */
int action_open(const char *path, const char *opener);

/*
 * Write the cd target to XFIND_CD_FILE and print a hint.
 * If path is a file, writes its parent directory.
 * Returns 0 on success, -1 on error.
 */
int action_cd(const char *path);

#endif /* ACTIONS_H */

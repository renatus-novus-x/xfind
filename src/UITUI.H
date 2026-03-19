/*
 * UITUI.H  --  Minimal TUI: candidate list, cursor, Enter/c/q
 */

#ifndef UITUI_H
#define UITUI_H

#include "MATCH.H"

/*
 * Show an interactive TUI for the given result set.
 *
 *   set    : search results to display
 *   opener : opener command override passed through to action_open()
 *            (NULL = use env / auto-detect / fallback)
 *
 * Keys:
 *   Up/Down or j/k  : move cursor
 *   Enter           : open selected entry
 *   c               : cd to selected entry (or its parent)
 *   q / ESC         : quit without action
 *
 * Returns 0 normally, -1 on error.
 */
int tui_run(const MatchSet *set, const char *opener);

#endif /* UITUI_H */

/*
 * UITUI.C  --  Minimal TUI implementation
 *
 * Uses ANSI escape sequences for cursor movement and clearing.
 * Both Ubuntu (any VT100 terminal) and X68K (IOCS console) support these.
 *
 * Raw mode:
 *   POSIX  : tcgetattr / tcsetattr
 *   X68K   : _dos_rawmode (or BIOS equivalent)
 */

#include "UITUI.H"
#include "ACTIONS.H"
#include "PLATFORM.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* ANSI escape helpers                                                  */
/* ------------------------------------------------------------------ */

#define ANSI_CLEAR_SCREEN  "\033[2J"
#define ANSI_HOME          "\033[H"
#define ANSI_CLEAR_LINE    "\033[2K"
#define ANSI_REVERSE       "\033[7m"
#define ANSI_RESET         "\033[0m"
#define ANSI_CURSOR_UP     "\033[A"
#define ANSI_CURSOR_DOWN   "\033[B"

/* Move cursor to row r (1-based), column 1 */
static void move_to(int row)
{
    printf("\033[%d;1H", row);
}

/* ------------------------------------------------------------------ */
/* Terminal raw mode                                                    */
/* ------------------------------------------------------------------ */

#ifdef PLATFORM_POSIX

#include <termios.h>
#include <unistd.h>

static struct termios g_saved;

static void raw_enter(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &g_saved);
    raw = g_saved;
    raw.c_lflag &= (unsigned)~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

static void raw_leave(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &g_saved);
}

static int read_key(void)
{
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == 0x1b) {
        /* Escape sequence */
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return 0x1b;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return 0x1b;
        if (seq[0] == '[') {
            if (seq[1] == 'A') return 'k'; /* up    -> k */
            if (seq[1] == 'B') return 'j'; /* down  -> j */
        }
        return 0x1b;
    }
    return (int)c;
}

#endif /* PLATFORM_POSIX */

#ifdef PLATFORM_X68K

static void raw_enter(void) { /* raw mode is default on X68K console */ }
static void raw_leave(void) {}

static int read_key(void)
{
    /* _dos_getchar_noecho: returns key without echo */
    int c = _dos_getchar_noecho();
    /* Map X68K special keys if needed (simplified) */
    return c;
}

#endif /* PLATFORM_X68K */

/* ------------------------------------------------------------------ */
/* Display helpers                                                      */
/* ------------------------------------------------------------------ */

#define HEADER_ROWS   2   /* title + separator */
#define MAX_VISIBLE  20   /* max entries shown at once */
#define MAX_PATH_DISP 72  /* truncated display width */

static void truncate_path(const char *path, char *out, int maxlen)
{
    int len = (int)strlen(path);
    if (len <= maxlen) {
        strncpy(out, path, (size_t)maxlen);
        out[maxlen] = '\0';
    } else {
        /* Show "...tail" */
        const char *tail = path + (len - (maxlen - 3));
        out[0] = '.'; out[1] = '.'; out[2] = '.';
        strncpy(out + 3, tail, (size_t)(maxlen - 3));
        out[maxlen] = '\0';
    }
}

static void draw_screen(const MatchSet *set, int cursor, int offset)
{
    int visible = set->count - offset;
    int i;
    char disp[MAX_PATH_DISP + 4];

    if (visible > MAX_VISIBLE) visible = MAX_VISIBLE;

    /* Clear screen and go home */
    printf(ANSI_CLEAR_SCREEN ANSI_HOME);

    /* Header */
    printf("xfind  [%d results]  Enter=open  c=cd  q=quit\n", set->count);
    printf("------------------------------------------------\n");

    for (i = 0; i < visible; i++) {
        int idx = offset + i;
        const IndexEntry *e = set->results[idx].entry;
        char type = e->type;

        move_to(HEADER_ROWS + 1 + i);
        printf(ANSI_CLEAR_LINE);

        truncate_path(e->path, disp, MAX_PATH_DISP);

        if (idx == cursor) {
            printf(ANSI_REVERSE " %c  %s" ANSI_RESET, type, disp);
        } else {
            printf("    %c  %s", type, disp);  /* 4 spaces indent */
        }
    }

    /* Status bar */
    move_to(HEADER_ROWS + MAX_VISIBLE + 2);
    printf(ANSI_CLEAR_LINE);
    if (set->count > 0) {
        printf("[%d/%d]", cursor + 1, set->count);
    }

    fflush(stdout);
}

/* ------------------------------------------------------------------ */
/* Main TUI loop                                                        */
/* ------------------------------------------------------------------ */

int tui_run(const MatchSet *set)
{
    int cursor = 0;
    int offset = 0;
    int done   = 0;
    int action = 0; /* 0=none 1=open 2=cd */

    if (set->count == 0) {
        printf("xfind: no results\n");
        return 0;
    }

    raw_enter();
    draw_screen(set, cursor, offset);

    while (!done) {
        int k = read_key();

        switch (k) {
        case 'q':
        case 0x1b: /* ESC */
            done = 1;
            break;

        case '\r':
        case '\n':
            action = 1; /* open */
            done   = 1;
            break;

        case 'c':
            action = 2; /* cd */
            done   = 1;
            break;

        case 'k': /* up */
        case 'K':
            if (cursor > 0) {
                cursor--;
                if (cursor < offset) offset = cursor;
            }
            break;

        case 'j': /* down */
        case 'J':
            if (cursor < set->count - 1) {
                cursor++;
                if (cursor >= offset + MAX_VISIBLE)
                    offset = cursor - MAX_VISIBLE + 1;
            }
            break;

        default:
            break;
        }

        if (!done) {
            draw_screen(set, cursor, offset);
        }
    }

    raw_leave();

    /* Clear TUI */
    printf(ANSI_CLEAR_SCREEN ANSI_HOME);
    fflush(stdout);

    if (action && cursor < set->count) {
        const char *path = set->results[cursor].entry->path;
        if (action == 1) return action_open(path);
        if (action == 2) return action_cd(path);
    }

    return 0;
}

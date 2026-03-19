/*
 * CONFIG.H  --  Minimal configuration file reader
 *
 * Format (INI-like):
 *   # comment
 *   index = /path/to/xfind.idx
 *   open_cmd = xdg-open
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "PLATFORM.H"

#define CONFIG_KEY_LEN   64
#define CONFIG_VAL_LEN   PATH_MAX_LEN

typedef struct ConfigEntry {
    char key[CONFIG_KEY_LEN];
    char val[CONFIG_VAL_LEN];
    struct ConfigEntry *next;
} ConfigEntry;

typedef struct {
    ConfigEntry *head;
} Config;

/*
 * Load config from file.  Returns 0 on success, -1 if file not found
 * (not a fatal error — caller may proceed with defaults).
 */
int         cfg_load(Config *cfg, const char *path);
const char *cfg_get(const Config *cfg, const char *key,
                    const char *default_val);
void        cfg_free(Config *cfg);

/* Default config file locations */
#define CONFIG_DEFAULT_NAME  "xfind.cfg"

#endif /* CONFIG_H */

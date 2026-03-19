/*
 * CONFIG.C  --  Configuration file reader implementation
 */

#include "CONFIG.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *trim(char *s)
{
    char *e;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) e--;
    *(e + 1) = '\0';
    return s;
}

int cfg_load(Config *cfg, const char *path)
{
    FILE *fp;
    char line[CONFIG_KEY_LEN + CONFIG_VAL_LEN + 8];

    cfg->head = NULL;

    fp = fopen(path, "r");
    if (!fp) return -1;

    while (fgets(line, sizeof(line), fp)) {
        char *p = trim(line);
        char *sep;
        ConfigEntry *ent;

        if (*p == '#' || *p == '\0') continue;

        sep = strchr(p, '=');
        if (!sep) continue;

        *sep = '\0';
        {
            char *k = trim(p);
            char *v = trim(sep + 1);

            ent = (ConfigEntry *)malloc(sizeof(ConfigEntry));
            if (!ent) break;

            strncpy(ent->key, k, CONFIG_KEY_LEN - 1);
            ent->key[CONFIG_KEY_LEN - 1] = '\0';
            strncpy(ent->val, v, CONFIG_VAL_LEN - 1);
            ent->val[CONFIG_VAL_LEN - 1] = '\0';

            ent->next  = cfg->head;
            cfg->head  = ent;
        }
    }

    fclose(fp);
    return 0;
}

const char *cfg_get(const Config *cfg, const char *key,
                    const char *default_val)
{
    const ConfigEntry *e = cfg->head;
    while (e) {
        if (strcmp(e->key, key) == 0) return e->val;
        e = e->next;
    }
    return default_val;
}

void cfg_free(Config *cfg)
{
    ConfigEntry *e = cfg->head;
    while (e) {
        ConfigEntry *next = e->next;
        free(e);
        e = next;
    }
    cfg->head = NULL;
}

#include "settings.h"

#include "config.h"
#include "utils/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SETTINGS_FILENAME "settings.cfg"

struct Settings settings;

bool defaultSettings(void)
{
    if (dirExists("/roms")) {
        sprintf(settings.dlPath, "/roms");
        settings.dlUseDirs = true; // The user probably wants to use platform-specific directories if has a roms directory
    } else {
        sprintf(settings.dlPath, "/");
        settings.dlUseDirs = false;
    }

    settings.colorScheme = COLOR_SCHEME_1;
    settings.lang = LANG_EN;
    settings.checkUpdateOnStart = true;

    return true;
}

bool loadSettings(void)
{
    FILE* fp = fopen(APPDATA_DIR "/" SETTINGS_FILENAME, "r");
    if (!fp)
        return false;

    char line[2048];
    char name[1024];
    char value[1024];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;

        if (sscanf(line, "%[^=]=%s", name, value) != 2) {
            fclose(fp);
            return false;
        }

        int valueInt = atoi(value);

        if (strcmp(name, "dlPath") == 0 && dirExists(value))
            snprintf(settings.dlPath, sizeof(settings.dlPath), value);
        else if (strcmp(name, "useDirs") == 0 && (valueInt == 0 || valueInt == 1))
            settings.dlUseDirs = valueInt;
        else if (strcmp(name, "colorScheme") == 0 && (valueInt > 0 && valueInt < COLOR_SCHEMES_COUNT + 1))
            settings.colorScheme = valueInt - 1;
        else if (strcmp(name, "lang") == 0 && (valueInt >= 0 && valueInt < LANGS_COUNT))
            settings.lang = valueInt;
        else if (strcmp(name, "checkUpdateOnStart") == 0 && (valueInt == 0 || valueInt == 1))
            settings.checkUpdateOnStart = valueInt;
    }

    fclose(fp);

    return true;
}

bool saveSettings(void)
{
    FILE* fp = fopen(APPDATA_DIR "/" SETTINGS_FILENAME, "w");
    if (!fp)
        return false;

    fprintf(fp, "dlPath=%s\n", settings.dlPath);
    fprintf(fp, "useDirs=%d\n", settings.dlUseDirs);
    fprintf(fp, "colorScheme=%d\n", settings.colorScheme + 1);
    fprintf(fp, "lang=%d\n", settings.lang);
    fprintf(fp, "checkUpdateOnStart=%d\n", settings.checkUpdateOnStart);

    fclose(fp);
    return true;
}

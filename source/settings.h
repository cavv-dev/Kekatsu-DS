#pragma once
#include "colors.h"
#include "gettext.h"
#include <sys/syslimits.h>

typedef enum {
    SETTING_DLPATH,
    SETTING_DLUSEDIRS,
    SETTING_COLORSCHEME,
    SETTING_CHECKUPDATEONSTART,
    SETTING_LANG,
    SETTINGS_COUNT
} SettingEnum;

struct Settings {
    char dlPath[PATH_MAX];
    bool dlUseDirs;
    ColorSchemeEnum colorScheme;
    LanguageEnum lang;
    bool checkUpdateOnStart;
};

extern struct Settings settings;

#define dlUseDirsStr(x) \
    ((x) == false ? "No" \
    : (x) == true ? "Yes" \
    : "")

#define langStr(x) \
    ((x) == LANG_EN ? "English" \
	: (x) == LANG_IT ? "Italian" \
    : (x) == LANG_NL ? "Dutch" \
	: "")

#define checkUpdateOnStartStr(x) \
    ((x) == false ? "No" \
    : (x) == true ? "Yes" \
    : "")

bool defaultSettings(void);
bool loadSettings(void);
bool saveSettings(void);

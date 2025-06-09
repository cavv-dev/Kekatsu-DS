#pragma once

typedef enum {
    LANG_EN,
    LANG_IT,
    LANG_NL,
    LANGS_COUNT
} LanguageEnum;

void loadLanguage(LanguageEnum lang);
const char* gettext(const char* msg);

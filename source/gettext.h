#pragma once

typedef enum {
    LANG_EN,
    LANG_IT,
    LANGS_COUNT
} LanguageEnum;

void loadLanguage(LanguageEnum lang);
const char* gettext(const char* msg);

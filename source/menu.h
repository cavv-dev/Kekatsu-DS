#pragma once

typedef enum {
    MENU_NONE,
    MENU_BROWSE,
    MENU_RESULTS,
    MENU_ENTRY,
    MENU_DOWNLOAD,
    MENU_DATABASES,
    MENU_SETTINGS,
    MENU_INFO,
    MENU_EXIT
} MenuEnum;

void menuBegin(MenuEnum);

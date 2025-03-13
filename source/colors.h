#pragma once
#include <calico/types.h>

typedef enum {
    COLOR_SCHEME_1,
    COLOR_SCHEME_2,
    COLOR_SCHEME_3,
    COLOR_SCHEME_4,
    COLOR_SCHEME_5,
    COLOR_SCHEME_6,
    COLOR_SCHEMES_COUNT
} ColorSchemeEnum;

typedef enum {
    COLOR_BG,
    COLOR_BG_2,
    COLOR_TEXT,
    COLOR_TEXT_2,
    COLOR_TEXT_3,
    COLOR_PRIMARY,
    COLOR_SECONDARY,
    COLORS_COUNT
} ColorEnum;

#define RGB15_8BIT(r, g, b) (((r) >> 3) | (((g) >> 3) << 5) | (((b) >> 3) << 10))

extern u16 colorSchemes[COLOR_SCHEMES_COUNT][COLORS_COUNT];

void initColorSchemes(void);

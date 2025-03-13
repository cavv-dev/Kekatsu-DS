#pragma once
#include <stdbool.h>

typedef struct GuiScreen* GuiScreen;

typedef enum {
    GUI_SCREEN_LCD_TOP,
    GUI_SCREEN_LCD_BOTTOM
} GuiScreenLcd;

typedef enum {
    GUI_ELEMENT_TYPE_BOX,
    GUI_ELEMENT_TYPE_IMAGE,
    GUI_ELEMENT_TYPE_BUTTON,
    GUI_ELEMENT_TYPE_TEXT,
    GUI_ELEMENT_TYPE_PROGRESSBAR,
    GUI_ELEMENT_TYPE_KEYBOARD
} GuiElementType;

GuiScreen newGuiScreen(GuiScreenLcd);
void freeGuiScreen(GuiScreen);
void addToGuiScreen(GuiScreen, void* element, GuiElementType);
void removeFromGuiScreen(GuiScreen, void* element);
void setGuiScreenDpadNavigate(GuiScreen, bool);
void setActiveScreens(GuiScreen topScreen, GuiScreen bottomScreen);
GuiScreen getActiveTopScreen(void);
GuiScreen getActiveBottomScreen(void);
void drawScreens(void);

#pragma once
#include "button.h"

typedef struct GuiKeyboard* GuiKeyboard;

typedef enum {
    GUI_KEYBOARD_EXTRA_KEY_NONE,
    GUI_KEYBOARD_EXTRA_KEY_BACKSPACE,
    GUI_KEYBOARD_EXTRA_KEY_SHIFT
} GuiKeyboardExtraKey;

struct GuiKeyboardKey {
    GuiBox bg;
    GuiBox bgHover;
    GuiText label;
    GuiButton btn;
    GuiImage icon;
    char c;
    GuiKeyboardExtraKey extraKey;
};

GuiKeyboard newGuiKeyboard(u16 textColor, u16 hoverColor);
void freeGuiKeyboard(GuiKeyboard);
void setGuiKeyboardPos(GuiKeyboard, size_t posX, size_t posY);
struct GuiKeyboardKey getGuiKeyboardPressed(GuiKeyboard);
void drawGuiKeyboard(GuiKeyboard);

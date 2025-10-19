#include "keyboard.h"

#include "backspaceKey_png.h"
#include "shiftKey_png.h"
#include <stdlib.h>

struct GuiKeyboard {
    size_t posX;
    size_t posY;
    struct GuiKeyboardKey* keys;
    bool shiftPressed;
};

#define KEYS_COUNT 43

void setKeys(GuiKeyboard gk)
{
    const char* chars;

    if (!gk->shiftPressed)
        chars = "1234567890\0qwertyuiop@asdfghjkl\0zxcvbnm-. \0";
    else
        chars = "1234567890\0QWERTYUIOP+ASDFGHJKL\0ZXCVBNM_/ \0";

    for (size_t i = 0; i < KEYS_COUNT; i++) {
        gk->keys[i].c = chars[i];
        gk->keys[i].extraKey = GUI_KEYBOARD_EXTRA_KEY_NONE;

        char text[2] = { chars[i], '\0' };
        setGuiTextText(gk->keys[i].label, text);
    }

    gk->keys[10].extraKey = GUI_KEYBOARD_EXTRA_KEY_BACKSPACE;
    gk->keys[31].extraKey = GUI_KEYBOARD_EXTRA_KEY_SHIFT;
}

void setKeysPos(GuiKeyboard gk)
{
    // First row
    for (size_t i = 0; i < 11; i++)
        setGuiButtonPos(gk->keys[i].btn, gk->posX + i * 23, gk->posY);

    // Second row
    for (size_t i = 11; i < 22; i++)
        setGuiButtonPos(gk->keys[i].btn, gk->posX + (i - 11) * 23 + 1, gk->posY + 23);

    // Third row
    for (size_t i = 22; i < 32; i++)
        setGuiButtonPos(gk->keys[i].btn, gk->posX + (i - 22) * 23 + 11, gk->posY + 23 * 2);

    // Fourth row
    for (size_t i = 32; i < 41; i++)
        setGuiButtonPos(gk->keys[i].btn, gk->posX + (i - 32) * 23 + 22, gk->posY + 23 * 3);

    // Space
    setGuiButtonPos(gk->keys[41].btn, gk->posX + 70, gk->posY + 23 * 4);
}

void setKeysShift(GuiKeyboard gk)
{
    gk->shiftPressed = (gk->shiftPressed ? false : true);
    setKeys(gk);
}

GuiKeyboard newGuiKeyboard(u16 textColor, u16 hoverColor)
{
    GuiKeyboard gk = malloc(sizeof(struct GuiKeyboard));
    gk->posX = 0;
    gk->posY = 0;
    gk->keys = malloc(sizeof(struct GuiKeyboardKey) * KEYS_COUNT);
    gk->shiftPressed = false;

    // All keys
    for (size_t i = 0; i < KEYS_COUNT; i++) {
        gk->keys[i].bg = newGuiBox(23, 23, 0);

        gk->keys[i].bgHover = newGuiBox(23, 23, 0);
        setGuiBoxBorder(gk->keys[i].bgHover, 1, hoverColor);

        gk->keys[i].label = newGuiText("", GUI_TEXT_SIZE_MEDIUM, textColor);
        setGuiTextAlignment(gk->keys[i].label, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

        gk->keys[i].btn = newGuiButton(23, 23);
        setGuiButtonBg(gk->keys[i].btn, gk->keys[i].bg, gk->keys[i].bgHover);
        setGuiButtonLabel(gk->keys[i].btn, gk->keys[i].label);
    }

    // Space
    setGuiBoxWidth(gk->keys[41].bg, 115);
    setGuiBoxBorder(gk->keys[41].bg, 1, textColor);
    setGuiBoxWidth(gk->keys[41].bgHover, 115);
    setGuiButtonWidth(gk->keys[41].btn, 115);

    // Backspace
    gk->keys[10].icon = newGuiImage(backspaceKey_pngBitmap, backspaceKey_pngPal, 15, 11, 16, 16, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(gk->keys[10].icon, textColor);
    setGuiButtonIcon(gk->keys[10].btn, gk->keys[10].icon, gk->keys[10].icon);

    // Shift
    gk->keys[31].icon = newGuiImage(shiftKey_pngBitmap, shiftKey_pngPal, 11, 13, 16, 16, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(gk->keys[31].icon, textColor);
    setGuiButtonIcon(gk->keys[31].btn, gk->keys[31].icon, gk->keys[31].icon);

    setKeysPos(gk);

    setKeys(gk);

    return gk;
}

void freeGuiKeyboard(GuiKeyboard gk)
{
    for (size_t i = 0; i < KEYS_COUNT; i++) {
        freeGuiBox(gk->keys[i].bg);
        freeGuiBox(gk->keys[i].bgHover);
        freeGuiText(gk->keys[i].label);
        freeGuiButton(gk->keys[i].btn);
    }

    freeGuiImage(gk->keys[10].icon); // Backspace
    freeGuiImage(gk->keys[31].icon); // Shift

    free(gk->keys);
    free(gk);
}

void setGuiKeyboardPos(GuiKeyboard gk, size_t posX, size_t posY)
{
    gk->posX = posX;
    gk->posY = posY;
    setKeysPos(gk);
}

struct GuiKeyboardKey getGuiKeyboardPressed(GuiKeyboard gk)
{
    for (size_t i = 0; i < KEYS_COUNT; i++) {
        if (getGuiButtonState(gk->keys[i].btn) != GUI_BUTTON_STATE_CLICKED)
            continue;

        if (i == 31) // Shift
            setKeysShift(gk);

        resetGuiButtonState(gk->keys[i].btn);
        return gk->keys[i];
    }

    return gk->keys[KEYS_COUNT - 1]; // None key
}

void drawGuiKeyboard(GuiKeyboard gk)
{
    for (size_t i = 0; i < KEYS_COUNT; i++) {
        handleTouchGuiButton(gk->keys[i].btn);
        drawGuiButton(gk->keys[i].btn);
    }
}

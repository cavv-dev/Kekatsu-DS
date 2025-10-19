#pragma once
#include <nds/ndstypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct GuiText* GuiText;

typedef enum {
    GUI_TEXT_SIZE_BIG,
    GUI_TEXT_SIZE_MEDIUM,
    GUI_TEXT_SIZE_SMALL
} GuiTextSize;

typedef enum {
    GUI_TEXT_H_ALIGN_LEFT,
    GUI_TEXT_H_ALIGN_CENTER,
    GUI_TEXT_H_ALIGN_RIGHT
} GuiTextHAlign;

typedef enum {
    GUI_TEXT_V_ALIGN_TOP,
    GUI_TEXT_V_ALIGN_MIDDLE,
    GUI_TEXT_V_ALIGN_BOTTOM
} GuiTextVAlign;

void initGuiFont(void);
GuiText newGuiText(const char* text, GuiTextSize, u16 color);
void freeGuiText(GuiText);
void setGuiTextText(GuiText, const char*);
void setGuiTextPos(GuiText, size_t posX, size_t posY);
void setGuiTextAlignment(GuiText, GuiTextHAlign, GuiTextVAlign);
void setGuiTextMaxWidth(GuiText, size_t);
void setGuiTextMaxHeight(GuiText, size_t);
void setGuiTextWrap(GuiText, bool);
size_t getGuiTextWidth(GuiText);
size_t getGuiTextHeight(GuiText);
size_t getGuiTextPosX(GuiText);
size_t getGuiTextPosY(GuiText);
GuiTextHAlign getGuiTextHAlignment(GuiText);
GuiTextVAlign getGuiTextVAlignment(GuiText);
void drawGuiTextPos(GuiText, size_t posX, size_t posY);
void drawGuiText(GuiText);

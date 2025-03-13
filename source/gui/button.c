#include "button.h"

#include "input.h"
#include <stdlib.h>

struct GuiButton {
    size_t width;
    size_t height;
    size_t posX;
    size_t posY;
    GuiButtonState state;
    GuiBox bg;
    GuiBox bgHover;
    GuiText label;
    GuiImage icon;
    GuiImage iconHover;
};

GuiButton newGuiButton(size_t width, size_t height)
{
    GuiButton gb = malloc(sizeof(struct GuiButton));
    gb->width = width;
    gb->height = height;
    gb->posX = 0;
    gb->posY = 0;
    gb->state = GUI_BUTTON_STATE_DEFAULT;
    gb->bg = NULL;
    gb->bgHover = NULL;
    gb->label = NULL;
    gb->icon = NULL;
    gb->iconHover = NULL;

    return gb;
}

void freeGuiButton(GuiButton gb)
{
    free(gb);
}

void setGuiButtonWidth(GuiButton gb, size_t width)
{
    gb->width = width;
}

void setGuiButtonHeight(GuiButton gb, size_t height)
{
    gb->height = height;
}

void setGuiButtonPos(GuiButton gb, size_t posX, size_t posY)
{
    gb->posX = posX;
    gb->posY = posY;
}

void setGuiButtonState(GuiButton gb, GuiButtonState state)
{
    gb->state = state;
}

void resetGuiButtonState(GuiButton gb)
{
    gb->state = GUI_BUTTON_STATE_DEFAULT;
}

void setGuiButtonBg(GuiButton gb, GuiBox bg, GuiBox bgHover)
{
    gb->bg = bg;
    gb->bgHover = bgHover;
}

void setGuiButtonLabel(GuiButton gb, GuiText label)
{
    gb->label = label;
}

void setGuiButtonIcon(GuiButton gb, GuiImage icon, GuiImage iconHover)
{
    gb->icon = icon;
    gb->iconHover = iconHover;
}

size_t getGuiButtonPosX(GuiButton gb)
{
    return gb->posX;
}

size_t getGuiButtonPosY(GuiButton gb)
{
    return gb->posY;
}

GuiButtonState getGuiButtonState(GuiButton gb)
{
    return gb->state;
}

void handleTouchGuiButton(GuiButton gb)
{
    if ((gb->state == GUI_BUTTON_STATE_HELD) && (touch.px == 0) && (touch.py == 0))
        gb->state = GUI_BUTTON_STATE_CLICKED;

    if (!touch.px & !touch.py)
        return;

    if (((touch.px >= gb->posX) && (touch.px <= (gb->posX + gb->width))) && ((touch.py >= gb->posY) && (touch.py <= (gb->posY + gb->height))))
        gb->state = GUI_BUTTON_STATE_HELD;
    else if (gb->state == GUI_BUTTON_STATE_HELD)
        gb->state = GUI_BUTTON_STATE_DEFAULT;
}

void drawGuiButtonPos(GuiButton gb, size_t posX, size_t posY)
{
    if (gb->bg && (gb->state == GUI_BUTTON_STATE_DEFAULT || gb->state == GUI_BUTTON_STATE_CLICKED))
        drawGuiBoxPos(gb->bg, posX, posY);
    else if (gb->bgHover && (gb->state == GUI_BUTTON_STATE_HELD || gb->state == GUI_BUTTON_STATE_SELECTED))
        drawGuiBoxPos(gb->bgHover, posX, posY);

    if (gb->icon && (gb->state == GUI_BUTTON_STATE_DEFAULT || gb->state == GUI_BUTTON_STATE_CLICKED))
        drawGuiImagePos(gb->icon, posX + (gb->width - getGuiImageWidth(gb->icon)) / 2, posY + (gb->height - getGuiImageHeight(gb->icon)) / 2);

    if (gb->iconHover && (gb->state == GUI_BUTTON_STATE_HELD || gb->state == GUI_BUTTON_STATE_SELECTED))
        drawGuiImagePos(gb->iconHover, posX + (gb->width - getGuiImageWidth(gb->iconHover)) / 2, posY + (gb->height - getGuiImageHeight(gb->iconHover)) / 2);

    if (gb->label) {
        size_t labelPosX;
        size_t labelPosY;

        switch (getGuiTextHAlignment(gb->label)) {
        case GUI_TEXT_H_ALIGN_CENTER:
            labelPosX = getGuiTextPosX(gb->label) + posX + gb->width / 2;
            break;
        case GUI_TEXT_H_ALIGN_RIGHT:
            labelPosX = getGuiTextPosX(gb->label) + posX + gb->width;
            break;
        default:
            labelPosX = getGuiTextPosX(gb->label) + posX;
            break;
        }

        switch (getGuiTextVAlignment(gb->label)) {
        case GUI_TEXT_V_ALIGN_MIDDLE:
            labelPosY = getGuiTextPosY(gb->label) + posY + gb->height / 2;
            break;
        case GUI_TEXT_V_ALIGN_BOTTOM:
            labelPosY = getGuiTextPosY(gb->label) + posY + getGuiTextHeight(gb->label);
            break;
        default:
            labelPosY = getGuiTextPosY(gb->label) + posY;
            break;
        }

        drawGuiTextPos(gb->label, labelPosX, labelPosY);
    }
}

void drawGuiButton(GuiButton gb)
{
    drawGuiButtonPos(gb, gb->posX, gb->posY);
}

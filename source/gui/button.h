#pragma once
#include "box.h"
#include "image.h"
#include "text.h"

typedef struct GuiButton* GuiButton;

typedef enum {
    GUI_BUTTON_STATE_DEFAULT,
    GUI_BUTTON_STATE_HELD,
    GUI_BUTTON_STATE_SELECTED,
    GUI_BUTTON_STATE_CLICKED
} GuiButtonState;

GuiButton newGuiButton(size_t width, size_t height);
void freeGuiButton(GuiButton);
void setGuiButtonWidth(GuiButton, size_t);
void setGuiButtonPos(GuiButton, size_t posX, size_t posY);
void setGuiButtonState(GuiButton, GuiButtonState);
void resetGuiButtonState(GuiButton);
void setGuiButtonBg(GuiButton, GuiBox bg, GuiBox bgHover);
void setGuiButtonLabel(GuiButton, GuiText);
void setGuiButtonIcon(GuiButton, GuiImage icon, GuiImage iconHover);
size_t getGuiButtonPosX(GuiButton);
size_t getGuiButtonPosY(GuiButton);
GuiButtonState getGuiButtonState(GuiButton);
void handleTouchGuiButton(GuiButton);
void drawGuiButtonPos(GuiButton, size_t posX, size_t posY);
void drawGuiButton(GuiButton);

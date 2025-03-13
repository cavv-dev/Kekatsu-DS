#pragma once
#include <calico/types.h>

typedef struct GuiBox* GuiBox;

GuiBox newGuiBox(size_t width, size_t height, u16 color);
void freeGuiBox(GuiBox gb);
void setGuiBoxWidth(GuiBox, size_t);
void setGuiBoxHeight(GuiBox, size_t);
void setGuiBoxPos(GuiBox, size_t posX, size_t posY);
void setGuiBoxBorder(GuiBox, size_t borderSize, u16 borderColor);
void drawGuiBoxPos(GuiBox, size_t posX, size_t posY);
void drawGuiBox(GuiBox);

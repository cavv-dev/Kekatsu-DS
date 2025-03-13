#pragma once
#include <calico/types.h>

typedef struct GuiProgressBar* GuiProgressbar;

GuiProgressbar newGuiProgressbar(size_t width, size_t height, u16 bgColor, u16 progressColor);
void freeGuiProgressbar(GuiProgressbar);
void setGuiProgressbarPos(GuiProgressbar, size_t posX, size_t posY);
void setGuiProgressbarPercent(GuiProgressbar, u8);
void drawGuiProgressbarPos(GuiProgressbar, size_t posX, size_t posY);
void drawGuiProgressbar(GuiProgressbar);

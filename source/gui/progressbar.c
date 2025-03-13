#include "progressbar.h"

#include <gl2d.h>

struct GuiProgressBar {
    size_t width;
    size_t height;
    size_t posX;
    size_t posY;
    u16 bgColor;
    u16 progressColor;
    u8 percent;
};

GuiProgressbar newGuiProgressbar(size_t width, size_t height, u16 bgColor, u16 progressColor)
{
    GuiProgressbar gp = malloc(sizeof(struct GuiProgressBar));
    gp->width = width;
    gp->height = height;
    gp->posX = 0;
    gp->posY = 0;
    gp->bgColor = bgColor;
    gp->progressColor = progressColor;
    gp->percent = 0;

    return gp;
}

void freeGuiProgressbar(GuiProgressbar gp)
{
    free(gp);
}

void setGuiProgressbarPos(GuiProgressbar gp, size_t posX, size_t posY)
{
    gp->posX = posX;
    gp->posY = posY;
}

void setGuiProgressbarPercent(GuiProgressbar gp, u8 percent)
{
    gp->percent = percent;
}

void drawGuiProgressbarPos(GuiProgressbar gp, size_t posX, size_t posY)
{
    glBoxFilled(
        posX,
        posY,
        posX + gp->width - 1,
        posY + gp->height - 1,
        gp->bgColor);

    if (gp->percent) {
        glBoxFilled(
            posX,
            posY,
            posX + (gp->width * gp->percent / 100) - 1,
            posY + gp->height - 1,
            gp->progressColor);
    }
}

void drawGuiProgressbar(GuiProgressbar gp)
{
    drawGuiProgressbarPos(gp, gp->posX, gp->posY);
}

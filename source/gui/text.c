#include "text.h"

#include "fontBig_0_png.h"
#include "fontMedium_0_png.h"
#include "fontSmall_0_png.h"
#include "gfx/fontBigUVCoords.h"
#include "gfx/fontMediumUVCoords.h"
#include "gfx/fontSmallUVCoords.h"
#include <gl2d.h>
#include <stdio.h>

glImage fontBig[FONTBIG_NUM_IMAGES];
glImage fontMedium[FONTBIG_NUM_IMAGES];
glImage fontSmall[FONTSMALL_NUM_IMAGES];

int fontBigTextureId;
int fontMediumTextureId;
int fontSmallTextureId;

struct GuiText {
    size_t width;
    size_t height;
    size_t posX;
    size_t posY;
    char* text;
    u16 color;
    size_t length;
    GuiTextHAlign hAlign;
    GuiTextVAlign vAlign;
    size_t maxWidth;
    size_t maxHeight;
    bool wrap;
    int textureId;
    glImage* font;
};

void initGuiFont(void)
{
    fontBigTextureId = glLoadSpriteSet(
        fontBig,
        FONTBIG_NUM_IMAGES,
        fontBigTexCoords,
        GL_RGB256,
        TEXTURE_SIZE_256,
        TEXTURE_SIZE_256,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        256,
        fontBig_0_pngPal,
        (u8*)fontBig_0_pngBitmap);

    fontMediumTextureId = glLoadSpriteSet(
        fontMedium,
        FONTMEDIUM_NUM_IMAGES,
        fontMediumTexCoords,
        GL_RGB256,
        TEXTURE_SIZE_256,
        TEXTURE_SIZE_256,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        256,
        fontMedium_0_pngPal,
        (u8*)fontMedium_0_pngBitmap);

    fontSmallTextureId = glLoadSpriteSet(
        fontSmall,
        FONTSMALL_NUM_IMAGES,
        fontSmallTexCoords,
        GL_RGB256,
        TEXTURE_SIZE_256,
        TEXTURE_SIZE_256,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        256,
        fontSmall_0_pngPal,
        (u8*)fontSmall_0_pngBitmap);
}

GuiText newGuiText(const char* text, GuiTextSize size, u16 color)
{
    GuiText gt = malloc(sizeof(struct GuiText));
    gt->width = 0;
    gt->height = 0;
    gt->posX = 0;
    gt->posY = 0;
    gt->text = NULL;
    gt->color = color;
    gt->hAlign = GUI_TEXT_H_ALIGN_LEFT;
    gt->vAlign = GUI_TEXT_V_ALIGN_TOP;
    gt->maxWidth = 0;
    gt->maxHeight = 0;
    gt->wrap = false;

    switch (size) {
    case GUI_TEXT_SIZE_BIG:
        gt->textureId = fontBigTextureId;
        gt->font = fontBig;
        break;
    case GUI_TEXT_SIZE_MEDIUM:
        gt->textureId = fontMediumTextureId;
        gt->font = fontMedium;
        break;
    case GUI_TEXT_SIZE_SMALL:
        gt->textureId = fontSmallTextureId;
        gt->font = fontSmall;
        break;
    }

    setGuiTextText(gt, text);

    return gt;
}

void freeGuiText(GuiText gt)
{
    free(gt->text);
    free(gt);
}

void updateWH(GuiText gt)
{
    size_t x = 0;
    size_t y = 0;

    size_t lineWidth = 0;
    size_t lineStart = 0;
    size_t lastSpace = -1;

    for (size_t i = 0; i < gt->length; i++) {
        glImage currChar = gt->font[gt->text[i] - 1];
        lineWidth += currChar.width + 1;

        if (gt->text[i] == ' ')
            lastSpace = i;

        if (gt->maxWidth && (lineWidth > gt->maxWidth || gt->text[i] == '\n')) {
            if (lineWidth > gt->maxWidth) {
                if (lastSpace != -1 && lastSpace > lineStart) {
                    i = lastSpace;
                    lineWidth = 0;

                    for (size_t j = lineStart; j <= lastSpace; j++)
                        lineWidth += gt->font[gt->text[j] - 1].width + 1;

                } else {
                    lineWidth -= currChar.width + 1;
                    i--;
                }
            }

            switch (gt->hAlign) {
            case GUI_TEXT_H_ALIGN_LEFT:
                x = 0;
                break;
            case GUI_TEXT_H_ALIGN_CENTER:
                x = 0 - lineWidth / 2;
                break;
            case GUI_TEXT_H_ALIGN_RIGHT:
                x = 0 - lineWidth;
                break;
            }

            for (size_t j = lineStart; j <= i; j++) {
                currChar = gt->font[gt->text[j] - 1];

                if (gt->text[j] != '\n')
                    x += currChar.width + 1;
            }

            lineStart = i + 1;
            lineWidth = 0;
            y += currChar.height + 1;
            lastSpace = -1;

            if (!gt->wrap || (gt->maxHeight && y + currChar.height > gt->maxHeight)) {
                gt->width = x;
                gt->height = y;
                return;
            }
        }
    }

    if (lineStart < gt->length) {
        switch (gt->hAlign) {
        case GUI_TEXT_H_ALIGN_LEFT:
            x = 0;
            break;
        case GUI_TEXT_H_ALIGN_CENTER:
            x = 0 - lineWidth / 2;
            break;
        case GUI_TEXT_H_ALIGN_RIGHT:
            x = 0 - lineWidth;
            break;
        }

        for (size_t j = lineStart; j < gt->length; j++) {
            glImage currChar = gt->font[gt->text[j] - 1];
            x += currChar.width + 1;
        }
    }

    gt->width = x;
    gt->height = y + gt->font[0].height + 1;
}

void setGuiTextPos(GuiText gt, size_t posX, size_t posY)
{
    gt->posX = posX;
    gt->posY = posY;
}

void setGuiTextText(GuiText gt, const char* text)
{
    free(gt->text);
    gt->text = strdup(text);
    gt->length = strlen(text);
    updateWH(gt);
}

void setGuiTextAlignment(GuiText gt, GuiTextHAlign hAlignment, GuiTextVAlign vAlignment)
{
    gt->hAlign = hAlignment;
    gt->vAlign = vAlignment;
}

void setGuiTextMaxWidth(GuiText gt, size_t maxWidth)
{
    gt->maxWidth = maxWidth;
    updateWH(gt);
}

void setGuiTextMaxHeight(GuiText gt, size_t maxHeight)
{
    gt->maxHeight = maxHeight;
    updateWH(gt);
}

void setGuiTextWrap(GuiText gt, bool wrap)
{
    gt->wrap = wrap;
    updateWH(gt);
}

size_t getGuiTextWidth(GuiText gt)
{
    return gt->width;
}

size_t getGuiTextHeight(GuiText gt)
{
    return gt->height;
}

size_t getGuiTextPosX(GuiText gt)
{
    return gt->posX;
}

size_t getGuiTextPosY(GuiText gt)
{
    return gt->posY;
}

GuiTextHAlign getGuiTextHAlignment(GuiText gt)
{
    return gt->hAlign;
}

GuiTextVAlign getGuiTextVAlignment(GuiText gt)
{
    return gt->vAlign;
}

void drawGuiTextPos(GuiText gt, size_t posX, size_t posY)
{
    size_t x = posX;
    size_t y = posY;

    size_t lineWidth = 0;
    size_t lineStart = 0;
    size_t lastSpace = -1;

    switch (gt->vAlign) {
    case GUI_TEXT_V_ALIGN_TOP:
        y = posY;
        break;
    case GUI_TEXT_V_ALIGN_MIDDLE:
        y = posY - gt->height / 2;
        break;
    case GUI_TEXT_V_ALIGN_BOTTOM:
        y = posY - gt->height;
        break;
    }

    size_t yStart = y;

    glSetActiveTexture(gt->textureId);
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(1));
    glColor(gt->color);

    for (size_t i = 0; i < gt->length; i++) {
        glImage currChar = gt->font[gt->text[i] - 1];
        lineWidth += currChar.width + 1;

        if (gt->text[i] == ' ')
            lastSpace = i;

        if (gt->maxWidth && (lineWidth > gt->maxWidth || gt->text[i] == '\n')) {
            if (lineWidth > gt->maxWidth) {
                if (lastSpace != -1 && lastSpace > lineStart) {
                    i = lastSpace;
                    lineWidth = 0;

                    for (size_t j = lineStart; j <= lastSpace; j++)
                        lineWidth += gt->font[gt->text[j] - 1].width + 1;

                } else {
                    lineWidth -= currChar.width + 1;
                    i--;
                }
            }

            switch (gt->hAlign) {
            case GUI_TEXT_H_ALIGN_LEFT:
                x = posX;
                break;
            case GUI_TEXT_H_ALIGN_CENTER:
                x = posX - lineWidth / 2;
                break;
            case GUI_TEXT_H_ALIGN_RIGHT:
                x = posX - lineWidth;
                break;
            }

            for (size_t j = lineStart; j <= i; j++) {
                currChar = gt->font[gt->text[j] - 1];

                if (gt->text[j] != '\n') {
                    glSprite(x, y, GL_FLIP_NONE, &currChar);
                    x += currChar.width + 1;
                }
            }

            lineStart = i + 1;
            lineWidth = 0;
            y += currChar.height + 1;
            lastSpace = -1;

            if (!gt->wrap || (gt->maxHeight && (y - yStart) + currChar.height > gt->maxHeight))
                return;
        }
    }

    if (lineStart < gt->length) {
        switch (gt->hAlign) {
        case GUI_TEXT_H_ALIGN_LEFT:
            x = posX;
            break;
        case GUI_TEXT_H_ALIGN_CENTER:
            x = posX - lineWidth / 2;
            break;
        case GUI_TEXT_H_ALIGN_RIGHT:
            x = posX - lineWidth;
            break;
        }

        for (size_t j = lineStart; j < gt->length; j++) {
            glImage currChar = gt->font[gt->text[j] - 1];
            glSprite(x, y, GL_FLIP_NONE, &currChar);
            x += currChar.width + 1;
        }
    }

    glColor(RGB15(31, 31, 31));
}

void drawGuiText(GuiText gt)
{
    drawGuiTextPos(gt, gt->posX, gt->posY);
}

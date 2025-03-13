#pragma once
#include <calico/types.h>

typedef struct GuiImage* GuiImage;

typedef enum {
    GUI_IMAGE_TEXTURE_TYPE_RGB,
    GUI_IMAGE_TEXTURE_TYPE_RGBA,
    GUI_IMAGE_TEXTURE_TYPE_RGB256
} GuiImageTextureType;

typedef enum {
    GUI_IMAGE_H_ALIGN_LEFT,
    GUI_IMAGE_H_ALIGN_CENTER,
    GUI_IMAGE_H_ALIGN_RIGHT
} GuiImageHAlign;

typedef enum {
    GUI_IMAGE_V_ALIGN_TOP,
    GUI_IMAGE_V_ALIGN_MIDDLE,
    GUI_IMAGE_V_ALIGN_BOTTOM
} GuiImageVAlign;

GuiImage newGuiImage(const unsigned* bitmap, const u16* pal, size_t width, size_t height, size_t bitmapWidth, size_t bitmapHeight, size_t resizeWidth, size_t resizeHeight, GuiImageTextureType textureType);
GuiImage newGuiImagePNG(const char* filePath, size_t resizeWidth, size_t resizeHeight);
void freeGuiImage(GuiImage);
void setGuiImagePos(GuiImage, size_t posX, size_t posY);
void setGuiImageAlign(GuiImage, GuiImageHAlign, GuiImageVAlign);
void setGuiImageColorTint(GuiImage, u16 color);
size_t getGuiImageWidth(GuiImage);
size_t getGuiImageHeight(GuiImage);
void drawGuiImagePos(GuiImage, size_t posX, size_t posY);
void drawGuiImage(GuiImage);

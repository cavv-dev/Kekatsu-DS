#include "image.h"

#include <gl2d.h>
#include <png.h>

struct GuiImage {
    size_t width;
    size_t height;
    size_t posX;
    size_t posY;
    int scale;
    GuiImageHAlign hAlign;
    GuiImageVAlign vAlign;
    glImage* image;
    int textureId;
    int colorTintPalId;
};

static void bitmapToRGB5A1(u8* bitmap, size_t width, size_t height, GuiImageTextureType textureType)
{
    size_t pixelCount = width * height;
    u16* rgb5a1Bitmap = (u16*)bitmap;

    for (size_t i = 0; i < pixelCount; i++) {
        u8 r = bitmap[i * (textureType == GUI_IMAGE_TEXTURE_TYPE_RGB ? 3 : 4)];
        u8 g = bitmap[i * (textureType == GUI_IMAGE_TEXTURE_TYPE_RGB ? 3 : 4) + 1];
        u8 b = bitmap[i * (textureType == GUI_IMAGE_TEXTURE_TYPE_RGB ? 3 : 4) + 2];
        u8 a = (textureType == GUI_IMAGE_TEXTURE_TYPE_RGB) ? 255 : bitmap[i * 4 + 3]; // Alpha is 255 if format is RGB

        u16 rgb5a1 = 0;
        rgb5a1 |= (r >> 3) & 0x1F; // 5 bits for red
        rgb5a1 |= ((g >> 3) & 0x1F) << 5; // 5 bits for green
        rgb5a1 |= ((b >> 3) & 0x1F) << 10; // 5 bits for blue
        rgb5a1 |= (a >> 7) << 15; // 1 bit for alpha (most significant bit of alpha)

        rgb5a1Bitmap[i] = rgb5a1;
    }
}

static bool pngFileToBitmap(const char* filePath, u8** bitmap, size_t* width, size_t* height, GuiImageTextureType* textureType)
{
    FILE* fp = fopen(filePath, "rb");
    if (!fp)
        return false;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return false;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);

    // 1024 is the max texture size supported
    if (*width > 1024 || *height > 1024) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return false;
    }

    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Convert to 8-bit depth if necessary
    if (bit_depth == 16)
        png_set_strip_16(png);
    else if (bit_depth < 8)
        png_set_packing(png);

    // Convert palette images to RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // Convert grayscale images to RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    // Add alpha channel if necessary
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    // Convert transparency to alpha
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    png_read_update_info(png, info);

    *bitmap = (u8*)malloc(png_get_rowbytes(png, info) * (*height));
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
    for (size_t y = 0; y < *height; y++)
        row_pointers[y] = *bitmap + y * png_get_rowbytes(png, info);

    png_read_image(png, row_pointers);

    *textureType = GUI_IMAGE_TEXTURE_TYPE_RGBA;

    bitmapToRGB5A1(*bitmap, *width, *height, *textureType);

    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
    free(row_pointers);

    return true;
}

// Calculate closest GL_TEXTURE_SIZE_ENUM
#define calculateGlTextureSizeEnum(x) \
    ((x) <= 8 ? TEXTURE_SIZE_8 \
    : (x) <= 16 ? TEXTURE_SIZE_16 \
    : (x) <= 32 ? TEXTURE_SIZE_32 \
    : (x) <= 64 ? TEXTURE_SIZE_64 \
    : (x) <= 128 ? TEXTURE_SIZE_128 \
    : (x) <= 256 ? TEXTURE_SIZE_256 \
    : (x) <= 512 ? TEXTURE_SIZE_512 \
    : (x) <= 1024 ? TEXTURE_SIZE_1024 \
    : 0)

GuiImage newGuiImage(const unsigned* bitmap, const u16* pal, size_t width, size_t height, size_t bitmapWidth, size_t bitmapHeight, size_t resizeWidth, size_t resizeHeight, GuiImageTextureType textureType)
{
    GuiImage gi = malloc(sizeof(struct GuiImage));
    gi->width = width;
    gi->height = height;
    gi->scale = 1 << 12;
    gi->hAlign = GUI_IMAGE_H_ALIGN_LEFT;
    gi->vAlign = GUI_IMAGE_V_ALIGN_TOP;
    gi->posX = 0;
    gi->posY = 0;
    gi->image = malloc(sizeof(glImage));
    gi->colorTintPalId = 0;

    size_t glTextureSizeWidth = calculateGlTextureSizeEnum(bitmapWidth);
    size_t glTextureSizeHeight = calculateGlTextureSizeEnum(bitmapHeight);

    GL_TEXTURE_TYPE_ENUM glTextureType;
    switch (textureType) {
    case GUI_IMAGE_TEXTURE_TYPE_RGB:
        glTextureType = GL_RGB;
        break;
    case GUI_IMAGE_TEXTURE_TYPE_RGBA:
        glTextureType = GL_RGBA;
        break;
    default:
        glTextureType = GL_RGB256;
        break;
    }

    gi->textureId = glLoadTileSet(
        gi->image,
        width,
        height,
        bitmapWidth,
        bitmapHeight,
        glTextureType,
        glTextureSizeWidth,
        glTextureSizeHeight,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        (textureType == GUI_IMAGE_TEXTURE_TYPE_RGB256 ? 256 : 0),
        pal,
        (u8*)bitmap);

    // Set scale to fit inside resized width and height
    if (resizeWidth || resizeHeight) {
        double scaleFactor = 1.0;

        if (resizeWidth && !resizeHeight) {
            scaleFactor = (double)resizeWidth / width;
            gi->width = resizeWidth;
            gi->height *= scaleFactor;
        } else if (!resizeWidth && resizeHeight) {
            scaleFactor = (double)resizeHeight / height;
            gi->height = resizeHeight;
            gi->width *= scaleFactor;
        } else {
            if (resizeWidth <= resizeHeight) {
                scaleFactor = (double)resizeWidth / width;
                gi->width = resizeWidth;
                gi->height *= scaleFactor;
            } else {
                scaleFactor = (double)resizeHeight / height;
                gi->height = resizeHeight;
                gi->width *= scaleFactor;
            }
        }

        gi->scale *= scaleFactor;
    }

    return gi;
}

GuiImage newGuiImagePNG(const char* filePath, size_t resizeWidth, size_t resizeHeight)
{
    unsigned* bitmap = NULL;
    GuiImageTextureType textureType;
    size_t width, height;

    if (!pngFileToBitmap(filePath, (u8**)&bitmap, &width, &height, &textureType))
        return NULL;

    GuiImage gi = newGuiImage(bitmap, NULL, width, height, width, height, resizeWidth, resizeHeight, textureType);
    free(bitmap);

    return gi;
}

void freeGuiImage(GuiImage gi)
{
    glDeleteTextures(1, &gi->textureId);

    if (gi->colorTintPalId)
        glDeleteTextures(1, &gi->colorTintPalId);

    free(gi->image);
    free(gi);
}

void setGuiImagePos(GuiImage gi, size_t posX, size_t posY)
{
    gi->posX = posX;
    gi->posY = posY;
}

void setGuiImageAlign(GuiImage gi, GuiImageHAlign hAlignment, GuiImageVAlign vAlignment)
{
    gi->hAlign = hAlignment;
    gi->vAlign = vAlignment;
}

void setGuiImageColorTint(GuiImage gi, u16 color)
{
    u16 colorTintPal[256];
    for (u16 i = 0; i < 256; i++)
        colorTintPal[i] = color;

    glGenTextures(1, &gi->colorTintPalId);
    glBindTexture(0, gi->colorTintPalId);
    glColorTableEXT(0, 0, 256, 0, 0, colorTintPal);
}

size_t getGuiImageWidth(GuiImage gi)
{
    return gi->width;
}

size_t getGuiImageHeight(GuiImage gi)
{
    return gi->height;
}

void drawGuiImagePos(GuiImage gi, size_t posX, size_t posY)
{
    size_t x;
    size_t y;

    switch (gi->hAlign) {
    case GUI_IMAGE_H_ALIGN_CENTER:
        x = posX - gi->width / 2;
        break;
    case GUI_IMAGE_H_ALIGN_RIGHT:
        x = posX - gi->width;
        break;
    default:
        x = posX;
        break;
    }

    switch (gi->vAlign) {
    case GUI_IMAGE_V_ALIGN_MIDDLE:
        y = posY - gi->height / 2;
        break;
    case GUI_IMAGE_V_ALIGN_BOTTOM:
        y = posY + gi->height;
        break;
    default:
        y = posY;
        break;
    }

    glSetActiveTexture(gi->textureId);

    if (gi->colorTintPalId)
        glAssignColorTable(0, gi->colorTintPalId);

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(1));

    glSpriteScale(x, y, gi->scale, GL_FLIP_NONE, gi->image);
}

void drawGuiImage(GuiImage gi)
{
    drawGuiImagePos(gi, gi->posX, gi->posY);
}

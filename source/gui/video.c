#include "video.h"

#include <gl2d.h>
#include <nds.h>

static void initSubSprites(void)
{
    oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);

    // Set up a 4x3 grid of 64x64 sprites to cover the screen
    u8 id = 0;
    for (u8 y = 0; y < 3; y++) {
        for (u8 x = 0; x < 4; x++, id++) {
            oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
            oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
            oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
        }
    }

    cothread_yield_irq(IRQ_VBLANK);
    oamUpdate(&oamSub);
}

void initGuiVideo(void)
{
    videoSetMode(MODE_0_3D);
    videoSetModeSub(MODE_5_2D);

    initSubSprites();
    bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

    glScreen2D();

    vramSetBankA(VRAM_A_TEXTURE_SLOT0);
    vramSetBankB(VRAM_B_TEXTURE_SLOT1);
    vramSetBankE(VRAM_E_TEX_PALETTE);
    vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
    vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5);
}

void guiLoop(void)
{
    glFlush(0);
    cothread_yield_irq(IRQ_VBLANK);
}

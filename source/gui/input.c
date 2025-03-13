#include "input.h"

u32 pressed;
touchPosition touch;

void updateInput(void)
{
    scanKeys();
    pressed = keysDown();

    touchRead(&touch);
}

#include "screen.h"

#include "button.h"
#include "input.h"
#include "keyboard.h"
#include "progressbar.h"
#include <gl2d.h>
#include <nds.h>

struct ElementNode {
    void* value;
    GuiElementType type;
    struct ElementNode* next;
};

struct ElementList {
    struct ElementNode* head;
    struct ElementNode* tail;
    struct ElementNode* selected;
    size_t size;
};

struct GuiScreen {
    GuiScreenLcd lcd;
    bool dpadNavigate;
    struct ElementList* elements;
};

static GuiScreen activeTopScreen = NULL;
static GuiScreen activeBottomScreen = NULL;

static GuiScreenLcd targetLcd = GUI_SCREEN_LCD_TOP;

GuiScreen newGuiScreen(GuiScreenLcd lcd)
{
    GuiScreen gs = malloc(sizeof(struct GuiScreen));
    gs->lcd = lcd;
    gs->dpadNavigate = true;

    struct ElementList* el = malloc(sizeof(struct ElementList));
    el->head = NULL;
    el->tail = NULL;
    el->selected = NULL;
    el->size = 0;
    gs->elements = el;

    return gs;
}

void freeGuiScreen(GuiScreen gs)
{
    struct ElementNode* curr = gs->elements->head;
    while (curr != NULL) {
        struct ElementNode* next = curr->next;
        free(curr);
        curr = next;
    }

    free(gs->elements);
    free(gs);
}

void setGuiScreenDpadNavigate(GuiScreen gs, bool dpadNavigate)
{
    gs->dpadNavigate = dpadNavigate;
}

void addToGuiScreen(GuiScreen gs, void* element, GuiElementType type)
{
    struct ElementNode* en = malloc(sizeof(struct ElementNode));
    en->value = element;
    en->type = type;
    en->next = NULL;

    if (gs->elements->head == NULL) {
        gs->elements->head = en;
        gs->elements->tail = en;
    } else {
        gs->elements->tail->next = en;
        gs->elements->tail = en;
    }

    gs->elements->size++;
}

void removeFromGuiScreen(GuiScreen gs, void* element)
{
    struct ElementNode* prev = NULL;
    struct ElementNode* curr = gs->elements->head;

    while (curr != NULL) {
        if (curr->value == element) {
            if (prev == NULL)
                gs->elements->head = curr->next;
            else
                prev->next = curr->next;

            if (curr == gs->elements->tail)
                gs->elements->tail = prev;

            free(curr);
            gs->elements->size--;
            break;
        }

        prev = curr;
        curr = curr->next;
    }
}

void drawGuiScreen(GuiScreen gs)
{
    glBegin2D();

    struct ElementNode* curr;
    curr = gs->elements->head;
    while (curr != NULL) {
        switch (curr->type) {
        case GUI_ELEMENT_TYPE_BOX:
            drawGuiBox(curr->value);
            break;
        case GUI_ELEMENT_TYPE_IMAGE:
            drawGuiImage(curr->value);
            break;
        case GUI_ELEMENT_TYPE_BUTTON:
            if (gs->lcd == GUI_SCREEN_LCD_BOTTOM)
                handleTouchGuiButton(curr->value);

            drawGuiButton(curr->value);
            break;
        case GUI_ELEMENT_TYPE_TEXT:
            drawGuiText(curr->value);
            break;
        case GUI_ELEMENT_TYPE_PROGRESSBAR:
            drawGuiProgressbar(curr->value);
            break;
        case GUI_ELEMENT_TYPE_KEYBOARD:
            drawGuiKeyboard(curr->value);
            break;
        }

        curr = curr->next;
    }

    glEnd2D();
}

void setActiveScreens(GuiScreen topScreen, GuiScreen bottomScreen)
{
    activeTopScreen = topScreen;
    activeBottomScreen = bottomScreen;
}

GuiScreen getActiveTopScreen(void)
{
    return activeTopScreen;
}

GuiScreen getActiveBottomScreen(void)
{
    return activeBottomScreen;
}

static void handleScreensDpadNavigate(void)
{
    if ((!activeTopScreen || !activeTopScreen->dpadNavigate) && (!activeBottomScreen || !activeBottomScreen->dpadNavigate))
        return;

    if (!(pressed & (KEY_DOWN | KEY_UP | KEY_LEFT | KEY_RIGHT | KEY_A)))
        return;

    GuiButton selected = NULL;
    size_t topScreenSize = activeTopScreen ? activeTopScreen->elements->size : 0;
    size_t bottomScreenSize = activeBottomScreen ? activeBottomScreen->elements->size : 0;
    GuiButton buttons[topScreenSize + bottomScreenSize];
    size_t buttonsCount = 0;

    struct ElementNode* curr;

    // Collect buttons from the top screen
    if (activeTopScreen && activeTopScreen->dpadNavigate) {
        curr = activeTopScreen->elements->head;
        for (; curr != NULL; curr = curr->next) {
            if (curr->type != GUI_ELEMENT_TYPE_BUTTON)
                continue;

            if (getGuiButtonState(curr->value) == GUI_BUTTON_STATE_SELECTED)
                selected = curr->value;

            buttons[buttonsCount++] = curr->value;
        }
    }

    // Collect buttons from the bottom screen
    if (activeBottomScreen && activeBottomScreen->dpadNavigate) {
        curr = activeBottomScreen->elements->head;
        for (; curr != NULL; curr = curr->next) {
            if (curr->type != GUI_ELEMENT_TYPE_BUTTON)
                continue;

            if (getGuiButtonState(curr->value) == GUI_BUTTON_STATE_SELECTED)
                selected = curr->value;

            buttons[buttonsCount++] = curr->value;
        }
    }

    if (buttonsCount == 0)
        return;

    // Find the highest button and select it if no button is selected
    if (!selected) {
        selected = buttons[0];
        for (size_t i = 1; i < buttonsCount; i++) {
            size_t selectedPosY = getGuiButtonPosY(selected) + (i < topScreenSize ? 192 : 0);
            size_t buttonPosY = getGuiButtonPosY(buttons[i]) + (i < topScreenSize ? 192 : 0);
            if (buttonPosY < selectedPosY)
                selected = buttons[i];
        }

        setGuiButtonState(selected, GUI_BUTTON_STATE_SELECTED);
        return;
    }

    // Handle button click
    if (pressed & KEY_A) {
        if (selected)
            setGuiButtonState(selected, GUI_BUTTON_STATE_CLICKED);

        return;
    }

    // Find the closest button according to the pressed key. Account for both screens
    GuiButton closest = NULL;
    for (size_t i = 0; i < buttonsCount; i++) {
        size_t selectedPosY = getGuiButtonPosY(selected) + (i < topScreenSize ? 192 : 0);
        size_t buttonPosY = getGuiButtonPosY(buttons[i]) + (i < topScreenSize ? 192 : 0);
        size_t selectedPosX = getGuiButtonPosX(selected);
        size_t buttonPosX = getGuiButtonPosX(buttons[i]);

        if (pressed & KEY_DOWN) {
            if (buttonPosY <= selectedPosY)
                continue;

            if (!closest || buttonPosY < getGuiButtonPosY(closest) + (i < topScreenSize ? 192 : 0))
                closest = buttons[i];
        } else if (pressed & KEY_UP) {
            if (buttonPosY >= selectedPosY)
                continue;

            if (!closest || buttonPosY > getGuiButtonPosY(closest) + (i < topScreenSize ? 192 : 0))
                closest = buttons[i];
        } else if (pressed & KEY_LEFT) {
            if (buttonPosX >= selectedPosX)
                continue;

            if (!closest || buttonPosX > getGuiButtonPosX(closest))
                closest = buttons[i];
        } else if (pressed & KEY_RIGHT) {
            if (buttonPosX <= selectedPosX)
                continue;

            if (!closest || buttonPosX < getGuiButtonPosX(closest))
                closest = buttons[i];
        }
    }

    // Update button states
    if (closest) {
        setGuiButtonState(selected, GUI_BUTTON_STATE_DEFAULT);
        setGuiButtonState(closest, GUI_BUTTON_STATE_SELECTED);
    }
}

void drawScreens(void)
{
    updateInput();

    handleScreensDpadNavigate();

    // Wait for capture unit to be ready
    while (REG_DISPCAPCNT & DCAP_ENABLE) { };

    if (activeTopScreen && activeBottomScreen) {
        if (targetLcd == GUI_SCREEN_LCD_TOP) {
            lcdMainOnBottom();
            vramSetBankC(VRAM_C_LCD);
            vramSetBankD(VRAM_D_SUB_SPRITE);
            REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);

            drawGuiScreen(activeTopScreen);
        } else {
            lcdMainOnTop();
            vramSetBankD(VRAM_D_LCD);
            vramSetBankC(VRAM_C_SUB_BG);
            REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);

            drawGuiScreen(activeBottomScreen);
        }

        // Swap target lcd
        targetLcd = targetLcd == GUI_SCREEN_LCD_TOP ? GUI_SCREEN_LCD_BOTTOM : GUI_SCREEN_LCD_TOP;
    } else if (activeTopScreen) {
        lcdMainOnTop();
        vramSetBankD(VRAM_D_LCD);
        vramSetBankC(VRAM_C_SUB_BG);
        REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);

        drawGuiScreen(activeTopScreen);
    } else {
        lcdMainOnBottom();
        vramSetBankC(VRAM_C_LCD);
        vramSetBankD(VRAM_D_SUB_SPRITE);
        REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);

        drawGuiScreen(activeBottomScreen);
    }
}

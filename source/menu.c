#include "menu.h"

#include "archives.h"
#include "brickColor1.h"
#include "brickColor2.h"
#include "colors.h"
#include "config.h"
#include "gettext.h"
#include "gui/box.h"
#include "gui/button.h"
#include "gui/image.h"
#include "gui/input.h"
#include "gui/keyboard.h"
#include "gui/progressbar.h"
#include "gui/screen.h"
#include "gui/text.h"
#include "gui/video.h"
#include "lButton.h"
#include "main.h"
#include "navbarBrowseIcon.h"
#include "navbarDatabasesIcon.h"
#include "navbarInfoIcon.h"
#include "navbarSettingsIcon.h"
#include "networking.h"
#include "rButton.h"
#include "settings.h"
#include "utils/filesystem.h"
#include "utils/math.h"
#include "utils/strings.h"
#include <nds.h>
#include <unistd.h>

// Current menu
MenuEnum menu;

// Choose color macro
#define col(x) colorSchemes[settings.colorScheme][x]

// Translate string macro
#define tr(x) gettext(x)

// Search parameters
char searchTitle[128];
size_t searchPage = 1;

Entry selectedEntry;

#define NAVBAR_BUTTONS_COUNT 4

struct Navbar {
    GuiBox bg;
    GuiBox btnBg;
    GuiBox btnBgHover;
    GuiImage btnIcons[NAVBAR_BUTTONS_COUNT];
    GuiImage btnIconsHover[NAVBAR_BUTTONS_COUNT];
    GuiButton btns[NAVBAR_BUTTONS_COUNT];
} navbar;

// Initializes the navbar with buttons and icons
void initNavbar(void)
{
    navbar.bg = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_SECONDARY));
    setGuiBoxPos(navbar.bg, 0, 160);

    navbar.btnBg = newGuiBox(64, 32, col(COLOR_SECONDARY));
    navbar.btnBgHover = newGuiBox(64, 32, col(COLOR_PRIMARY));

    navbar.btnIcons[0] = newGuiImage(navbarBrowseIconBitmap, navbarBrowseIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    navbar.btnIcons[1] = newGuiImage(navbarDatabasesIconBitmap, navbarDatabasesIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    navbar.btnIcons[2] = newGuiImage(navbarSettingsIconBitmap, navbarSettingsIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    navbar.btnIcons[3] = newGuiImage(navbarInfoIconBitmap, navbarInfoIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);

    navbar.btnIconsHover[0] = newGuiImage(navbarBrowseIconBitmap, navbarBrowseIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    navbar.btnIconsHover[1] = newGuiImage(navbarDatabasesIconBitmap, navbarDatabasesIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    navbar.btnIconsHover[2] = newGuiImage(navbarSettingsIconBitmap, navbarSettingsIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    navbar.btnIconsHover[3] = newGuiImage(navbarInfoIconBitmap, navbarInfoIconPal, 20, 20, 32, 32, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);

    for (size_t i = 0; i < NAVBAR_BUTTONS_COUNT; i++) {
        setGuiImageColorTint(navbar.btnIcons[i], col(COLOR_PRIMARY));
        setGuiImageColorTint(navbar.btnIconsHover[i], col(COLOR_SECONDARY));

        navbar.btns[i] = newGuiButton(64, 32);
        setGuiButtonBg(navbar.btns[i], navbar.btnBg, navbar.btnBgHover);
        setGuiButtonIcon(navbar.btns[i], navbar.btnIcons[i], navbar.btnIconsHover[i]);
        setGuiButtonPos(navbar.btns[i], i * 64, 160);
    }
}

// Frees the resources allocated for the navbar
void freeNavbar(void)
{
    freeGuiBox(navbar.bg);
    freeGuiBox(navbar.btnBgHover);
    freeGuiImage(navbar.btnIcons[0]);
    freeGuiImage(navbar.btnIcons[1]);
    freeGuiImage(navbar.btnIcons[2]);
    freeGuiImage(navbar.btnIcons[3]);
    freeGuiImage(navbar.btnIconsHover[0]);
    freeGuiImage(navbar.btnIconsHover[1]);
    freeGuiImage(navbar.btnIconsHover[2]);
    freeGuiImage(navbar.btnIconsHover[3]);

    for (size_t i = 0; i < NAVBAR_BUTTONS_COUNT; i++)
        freeGuiButton(navbar.btns[i]);
}

// Switches the current menu and updates the navbar buttons accordingly
void switchMenu(MenuEnum targetMenu)
{
    // Set navbar buttons to original state
    for (size_t i = 0; i < NAVBAR_BUTTONS_COUNT; i++) {
        setGuiButtonBg(navbar.btns[i], navbar.btnBg, navbar.btnBgHover);
        setGuiButtonIcon(navbar.btns[i], navbar.btnIcons[i], navbar.btnIconsHover[i]);
        resetGuiButtonState(navbar.btns[i]);
    }

    switch (targetMenu) {
    case MENU_BROWSE:
        setGuiButtonBg(navbar.btns[0], navbar.btnBgHover, navbar.btnBgHover);
        setGuiButtonIcon(navbar.btns[0], navbar.btnIconsHover[0], navbar.btnIconsHover[0]);
        break;
    case MENU_DATABASES:
        setGuiButtonBg(navbar.btns[1], navbar.btnBgHover, navbar.btnBgHover);
        setGuiButtonIcon(navbar.btns[1], navbar.btnIconsHover[1], navbar.btnIconsHover[1]);
        break;
    case MENU_SETTINGS:
        setGuiButtonBg(navbar.btns[2], navbar.btnBgHover, navbar.btnBgHover);
        setGuiButtonIcon(navbar.btns[2], navbar.btnIconsHover[2], navbar.btnIconsHover[2]);
        break;
    case MENU_INFO:
        setGuiButtonBg(navbar.btns[3], navbar.btnBgHover, navbar.btnBgHover);
        setGuiButtonIcon(navbar.btns[3], navbar.btnIconsHover[3], navbar.btnIconsHover[3]);
        break;
    default:
        break;
    }

    menu = targetMenu;
}

// Switches the menu based on which navbar button has been clicked
void navbarSwitchMenu(void)
{
    for (size_t i = 0; i < NAVBAR_BUTTONS_COUNT; i++) {
        if (getGuiButtonState(navbar.btns[i]) != GUI_BUTTON_STATE_CLICKED)
            continue;

        switch (i) {
        case 0:
            switchMenu(MENU_BROWSE);
            break;
        case 1:
            switchMenu(MENU_DATABASES);
            break;
        case 2:
            switchMenu(MENU_SETTINGS);
            break;
        case 3:
            switchMenu(MENU_INFO);
            break;
        }

        resetGuiButtonState(navbar.btns[i]);
    }
}

// Adds the navbar elements to the given screen
void addNavbarToGuiScreen(GuiScreen gs)
{
    addToGuiScreen(gs, navbar.bg, GUI_ELEMENT_TYPE_BOX);

    for (size_t i = 0; i < NAVBAR_BUTTONS_COUNT; i++)
        addToGuiScreen(gs, navbar.btns[i], GUI_ELEMENT_TYPE_BUTTON);
}

// Displays a keyboard interface to edit the given string
// Returns true if the changes to the string have been saved, false if ignored
bool keyboardEdit(char* str, size_t maxLength)
{
    char tempStr[maxLength];
    strncpy(tempStr, str, maxLength);

    size_t pos = strlen(tempStr); // Char write position

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox topBg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));
    GuiBox bottomBg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG_2));
    GuiBox buttonsBg = newGuiBox(SCREEN_WIDTH, 50, col(COLOR_SECONDARY));
    setGuiBoxPos(buttonsBg, 0, 142);

    GuiBox textboxBg = newGuiBox(224, 80, col(COLOR_BG_2));
    setGuiBoxBorder(textboxBg, 2, col(COLOR_PRIMARY));
    setGuiBoxPos(textboxBg, 16, 56);

    GuiText textboxTxt = newGuiText(tempStr, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(textboxTxt, 128, 96);
    setGuiTextAlignment(textboxTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(textboxTxt, 214);
    setGuiTextMaxHeight(textboxTxt, 64);
    setGuiTextWrap(textboxTxt, true);

    GuiKeyboard keyboard = newGuiKeyboard(col(COLOR_TEXT_2), col(COLOR_PRIMARY));
    setGuiKeyboardPos(keyboard, 0, 16);

    GuiBox btnBg = newGuiBox(70, 28, col(COLOR_BG_2));
    setGuiBoxBorder(btnBg, 2, col(COLOR_PRIMARY));
    GuiBox btnBgHover = newGuiBox(70, 28, col(COLOR_PRIMARY));

    GuiText btnTxt[2];
    btnTxt[0] = newGuiText(tr("Ok"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    btnTxt[1] = newGuiText(tr("Cancel"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextAlignment(btnTxt[0], GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextAlignment(btnTxt[1], GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiButton btn[2];
    btn[0] = newGuiButton(70, 28);
    btn[1] = newGuiButton(70, 28);
    setGuiButtonBg(btn[0], btnBg, btnBgHover);
    setGuiButtonBg(btn[1], btnBg, btnBgHover);
    setGuiButtonLabel(btn[0], btnTxt[0]);
    setGuiButtonLabel(btn[1], btnTxt[1]);
    setGuiButtonPos(btn[0], 45, 152);
    setGuiButtonPos(btn[1], 141, 152);

    addToGuiScreen(topScreen, topBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, textboxBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, textboxTxt, GUI_ELEMENT_TYPE_TEXT);

    addToGuiScreen(bottomScreen, bottomBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, keyboard, GUI_ELEMENT_TYPE_KEYBOARD);
    addToGuiScreen(bottomScreen, buttonsBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, btn[0], GUI_ELEMENT_TYPE_BUTTON);
    addToGuiScreen(bottomScreen, btn[1], GUI_ELEMENT_TYPE_BUTTON);

    GuiScreen prevTopScreen = getActiveTopScreen();
    GuiScreen prevBottomScreen = getActiveBottomScreen();

    setActiveScreens(topScreen, bottomScreen);

    bool save = false;
    while (pmMainLoop()) {
        guiLoop();
        drawScreens();

        struct GuiKeyboardKey key = getGuiKeyboardPressed(keyboard);

        if (key.c && pos + 1 <= maxLength) {
            tempStr[pos] = key.c;
            tempStr[++pos] = '\0';
            setGuiTextText(textboxTxt, tempStr);
        } else if (key.extraKey == GUI_KEYBOARD_EXTRA_KEY_BACKSPACE) {
            if (pos > 0)
                tempStr[--pos] = '\0';
            setGuiTextText(textboxTxt, tempStr);
        }

        if (getGuiButtonState(btn[0]) == GUI_BUTTON_STATE_CLICKED) {
            save = true;
            strncpy(str, tempStr, maxLength - 1);
            break;
        } else if (getGuiButtonState(btn[1]) == GUI_BUTTON_STATE_CLICKED) {
            save = false;
            break;
        }
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(topBg);
    freeGuiBox(bottomBg);
    freeGuiBox(buttonsBg);
    freeGuiBox(textboxBg);
    freeGuiText(textboxTxt);
    freeGuiKeyboard(keyboard);
    freeGuiBox(btnBg);
    freeGuiBox(btnBgHover);
    freeGuiText(btnTxt[0]);
    freeGuiText(btnTxt[1]);
    freeGuiButton(btn[0]);
    freeGuiButton(btn[1]);

    setActiveScreens(prevTopScreen, prevBottomScreen);

    return save;
}

// Displays a prompt window with the given title and message
// Returns true if the first option is clicked, false if the second option is clicked
bool windowPrompt(const char* title, const char* message, const char* btn1Label, const char* btn2Label)
{
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiBox promptBg = newGuiBox(244, 132, col(COLOR_BG_2));
    setGuiBoxBorder(promptBg, 2, col(COLOR_PRIMARY));
    setGuiBoxPos(promptBg, 6, 6);

    GuiText titleTxt = newGuiText(title, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(titleTxt, 128, 25);
    setGuiTextAlignment(titleTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText messageTxt = newGuiText(message, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(messageTxt, 128, 78);
    setGuiTextAlignment(messageTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(messageTxt, 224);
    setGuiTextMaxHeight(messageTxt, 100);
    setGuiTextWrap(messageTxt, true);

    GuiText btn1Txt = NULL;
    GuiText btn2Txt = NULL;
    GuiBox btnBg1 = NULL;
    GuiBox btnBg1Hover = NULL;
    GuiBox btnBg2 = NULL;
    GuiBox btnBg2Hover = NULL;
    GuiButton btn1 = NULL;
    GuiButton btn2 = NULL;

    btn1Txt = newGuiText(btn1Label, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextAlignment(btn1Txt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    size_t btn1Width = max(70, getGuiTextWidth(btn1Txt) + 20);

    btnBg1 = newGuiBox(btn1Width, 28, col(COLOR_BG_2));
    setGuiBoxBorder(btnBg1, 2, col(COLOR_PRIMARY));
    btnBg1Hover = newGuiBox(btn1Width, 28, col(COLOR_PRIMARY));

    btn1 = newGuiButton(btn1Width, 28);
    setGuiButtonBg(btn1, btnBg1, btnBg1Hover);
    setGuiButtonLabel(btn1, btn1Txt);
    setGuiButtonPos(btn1, 93, 152);

    if (btn2Label) {
        btn2Txt = newGuiText(btn2Label, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
        setGuiTextAlignment(btn2Txt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

        size_t btn2Width = max(70, getGuiTextWidth(btn2Txt) + 20);

        btnBg2 = newGuiBox(btn2Width, 28, col(COLOR_BG_2));
        setGuiBoxBorder(btnBg2, 2, col(COLOR_PRIMARY));
        btnBg2Hover = newGuiBox(btn2Width, 28, col(COLOR_PRIMARY));

        btn2 = newGuiButton(btn2Width, 28);
        setGuiButtonBg(btn2, btnBg2, btnBg2Hover);
        setGuiButtonLabel(btn2, btn2Txt);

        setGuiButtonPos(btn1, 80 - btn1Width / 2, 152);
        setGuiButtonPos(btn2, 176 - btn2Width / 2, 152);
    }

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, promptBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, titleTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, messageTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, btn1, GUI_ELEMENT_TYPE_BUTTON);
    if (btn2Label)
        addToGuiScreen(bottomScreen, btn2, GUI_ELEMENT_TYPE_BUTTON);

    GuiScreen prevTopScreen = getActiveTopScreen();
    GuiScreen prevBottomScreen = getActiveBottomScreen();

    setActiveScreens(NULL, bottomScreen);

    bool ret = true;
    while (pmMainLoop()) {
        guiLoop();
        drawScreens();

        if (getGuiButtonState(btn1) == GUI_BUTTON_STATE_CLICKED)
            break;

        if (btn2Label && getGuiButtonState(btn2) == GUI_BUTTON_STATE_CLICKED) {
            ret = false;
            break;
        }
    }

    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiBox(promptBg);
    freeGuiText(titleTxt);
    freeGuiText(messageTxt);
    freeGuiText(btn1Txt);
    freeGuiBox(btnBg1);
    freeGuiBox(btnBg1Hover);
    freeGuiButton(btn1);

    if (btn2Label) {
        freeGuiText(btn2Txt);
        freeGuiBox(btnBg2);
        freeGuiBox(btnBg2Hover);
        freeGuiButton(btn2);
    }

    setActiveScreens(prevTopScreen, prevBottomScreen);

    return ret;
}

// Displays a message on the bottom screen in a non-blocking way
void showMessage(const char* message)
{
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiText messageTxt = newGuiText(message, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(messageTxt, 128, 96);
    setGuiTextAlignment(messageTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(messageTxt, 244);
    setGuiTextWrap(messageTxt, true);

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, messageTxt, GUI_ELEMENT_TYPE_TEXT);

    GuiScreen prevTopScreen = getActiveTopScreen();
    GuiScreen prevBottomScreen = getActiveBottomScreen();

    setActiveScreens(NULL, bottomScreen);

    // Make sure to draw bottom screen
    guiLoop();
    drawScreens();
    guiLoop();
    drawScreens();

    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiText(messageTxt);

    setActiveScreens(prevTopScreen, prevBottomScreen);
}

// Draws a black background on both screens
void drawBlank(void)
{
    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    addToGuiScreen(topScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);

    setActiveScreens(topScreen, bottomScreen);

    // Make sure to draw both screens
    guiLoop();
    drawScreens();
    guiLoop();
    drawScreens();

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
}

// Downloads and loads a box art file of a given entry into a new GuiImage element
// Returns the created GuiImage element
GuiImage loadBoxart(Entry e)
{
    char boxartFilename[NAME_MAX];
    strncpy(boxartFilename, getEntryBoxartUrl(e), sizeof(boxartFilename) - 1);
    safeStr(boxartFilename);

    char boxartPath[PATH_MAX];
    joinPath(boxartPath, CACHE_DIR, boxartFilename);

    if (!fileExists(boxartPath))
        downloadFile(boxartPath, getEntryBoxartUrl(e), NULL);

    return newGuiImagePNG(boxartPath, 144, 144);
}

// Data and elements used dynamically by the downloadProgressCallback function
struct DlData {
    GuiText fileNameTxt;
    GuiProgressbar progressBar;
    GuiText percentTxt;
    GuiText speedTxt;
    GuiText progressTxt;
    GuiButton cancelBtn;
    time_t lastSpeedUpdateTime;
    size_t lastDlnow;
} dlData;

// curl download progress function
// Draws the download progress interface
size_t downloadProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    guiLoop();

    char humanSizeDlnow[16];
    char humanSizeDltotal[16];

    humanizeSize(humanSizeDlnow, sizeof(humanSizeDlnow), dlnow);
    humanizeSize(humanSizeDltotal, sizeof(humanSizeDltotal), dltotal);

    char progressText[64];
    char percentText[16];

    sprintf(progressText, "%s / %s", humanSizeDlnow, humanSizeDltotal);

    size_t percent = 0;
    if (dltotal) {
        percent = (double)dlnow / dltotal * 100;
        sprintf(percentText, "%d %%", percent);
    } else {
        percentText[0] = '\0';
    }

    setGuiProgressbarPercent(dlData.progressBar, percent);
    setGuiTextText(dlData.percentTxt, percentText);
    setGuiTextText(dlData.progressTxt, progressText);

    time_t currentTime = time(NULL);

    // Update speed if 1 second elapsed
    if ((currentTime - dlData.lastSpeedUpdateTime >= 1)) {
        char humanSizeSpeed[16];
        humanizeSize(humanSizeSpeed, sizeof(humanSizeSpeed), dlnow - dlData.lastDlnow);

        char speedText[32];
        snprintf(speedText, sizeof(speedText), "%s/s", humanSizeSpeed);

        setGuiTextText(dlData.speedTxt, speedText);
        dlData.lastSpeedUpdateTime = currentTime;
        dlData.lastDlnow = dlnow;
    }

    drawScreens();

    if (getGuiButtonState(dlData.cancelBtn) == GUI_BUTTON_STATE_CLICKED) {
        resetGuiButtonState(dlData.cancelBtn);
        stopDownload();
    }

    return 0;
}

// Sets up the download progress interface and starts the download
// Returns the download status
DownloadStatus downloadGui(const char* path, const char* url, const char* fileName)
{
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    dlData.fileNameTxt = newGuiText(fileName, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(dlData.fileNameTxt, 14, 56);
    setGuiTextAlignment(dlData.fileNameTxt, GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_BOTTOM);
    setGuiTextMaxWidth(dlData.fileNameTxt, 216);
    setGuiTextWrap(dlData.fileNameTxt, true);
    setGuiTextMaxHeight(dlData.fileNameTxt, 60);

    GuiBox progressBarBg = newGuiBox(228, 26, col(COLOR_BG_2));
    setGuiBoxBorder(progressBarBg, 2, col(COLOR_PRIMARY));
    setGuiBoxPos(progressBarBg, 12, 60);
    dlData.progressBar = newGuiProgressbar(224, 22, col(COLOR_BG_2), col(COLOR_PRIMARY));
    setGuiProgressbarPos(dlData.progressBar, 14, 62);

    dlData.percentTxt = newGuiText("", GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(dlData.percentTxt, 128, 73);
    setGuiTextAlignment(dlData.percentTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    dlData.speedTxt = newGuiText("", GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(dlData.speedTxt, 14, 92);

    dlData.progressTxt = newGuiText("", GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(dlData.progressTxt, 242, 92);
    setGuiTextAlignment(dlData.progressTxt, GUI_TEXT_H_ALIGN_RIGHT, GUI_TEXT_V_ALIGN_TOP);

    GuiBox cancelBtnBg = newGuiBox(90, 34, col(COLOR_BG_2));
    setGuiBoxBorder(cancelBtnBg, 2, col(COLOR_PRIMARY));
    GuiBox cancelBtnBgHover = newGuiBox(90, 34, col(COLOR_PRIMARY));
    GuiText cancelBtnTxt = newGuiText(tr("Cancel"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextAlignment(cancelBtnTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    dlData.cancelBtn = newGuiButton(90, 34);
    setGuiButtonPos(dlData.cancelBtn, 83, 124);
    setGuiButtonBg(dlData.cancelBtn, cancelBtnBg, cancelBtnBgHover);
    setGuiButtonLabel(dlData.cancelBtn, cancelBtnTxt);

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, dlData.fileNameTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, progressBarBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, dlData.progressBar, GUI_ELEMENT_TYPE_PROGRESSBAR);
    addToGuiScreen(bottomScreen, dlData.percentTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, dlData.speedTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, dlData.progressTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, dlData.cancelBtn, GUI_ELEMENT_TYPE_BUTTON);

    setActiveScreens(NULL, bottomScreen);

    dlData.lastSpeedUpdateTime = 0;
    dlData.lastDlnow = 0;
    DownloadStatus dlStatus = downloadFile(path, url, downloadProgressCallback);

    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiText(dlData.fileNameTxt);
    freeGuiBox(progressBarBg);
    freeGuiProgressbar(dlData.progressBar);
    freeGuiText(dlData.percentTxt);
    freeGuiText(dlData.speedTxt);
    freeGuiText(dlData.progressTxt);
    freeGuiBox(cancelBtnBg);
    freeGuiBox(cancelBtnBgHover);
    freeGuiText(cancelBtnTxt);
    freeGuiButton(dlData.cancelBtn);

    return dlStatus;
}

// Handles the download of the given entry
void downloadEntry(Entry e)
{
    char downloadDirPath[PATH_MAX];
    char downloadFilePath[PATH_MAX];
    char tempMessage[128];

    if (settings.dlUseDirs)
        joinPath(downloadDirPath, settings.dlPath, getEntryPlatform(e));
    else
        strcpy(downloadDirPath, settings.dlPath);

    joinPath(downloadFilePath, downloadDirPath, getEntryFileName(e));

    if (!createDir(downloadDirPath)) {
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Download failed"), tr("could not create download directory"));
        windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
        return;
    }

    DownloadStatus dlStatus = downloadGui(downloadFilePath, getEntryUrl(e), getEntryFileName(e));

    switch (dlStatus) {
    case DOWNLOAD_STOPPED:
        return;
    case DOWNLOAD_ERR_NOT_OK:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Download failed"), tr("bad response status"));
        windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
        return;
    case DOWNLOAD_ERR_INIT_FAILED:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Download failed"), tr("could not initialize download"));
        windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
        return;
    case DOWNLOAD_ERR_FILE_OPEN_FAILED:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Download failed"), tr("could not create file"));
        windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
        return;
    case DOWNLOAD_ERR_PERFORM_FAILED:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Download failed"), tr("could not perform download"));
        windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
        return;
    default:
        break;
    }

    if (!fileIsZip(downloadFilePath)) {
        snprintf(tempMessage, sizeof(tempMessage), "%s\n(%s)", tr("Download completed"), getEntryTitle(e));
        windowPrompt(tr("Info"), tempMessage, tr("Ok"), NULL);
        return;
    }

    ExtractStatus extractStatus;
    char tempExtractPath[PATH_MAX];

    size_t extractItemsCount = getEntryExtractItemsCount(e);
    if (extractItemsCount) {
        struct EntryExtractItem* extractItems = getEntryExtractItems(e);
        for (size_t i = 0; i < extractItemsCount; i++) {
            joinPath(tempExtractPath, downloadDirPath, extractItems[i].outPath);

            snprintf(tempMessage, sizeof(tempMessage), "%s (%s)", tr("Extracting content..."), extractItems[i].inPath);
            showMessage(tempMessage);

            extractStatus = extractZip(downloadFilePath, extractItems[i].inPath, tempExtractPath);

            switch (extractStatus) {
            case EXTRACT_ERR_FILE_OPEN:
                snprintf(tempMessage, sizeof(tempMessage), "%s: %s (%s)", tr("Extraction failed"), tr("could not open file"), extractItems[i].inPath);
                windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
                break;
            case EXTRACT_ERR_FILE_NOT_FOUND:
                snprintf(tempMessage, sizeof(tempMessage), "%s: %s (%s)", tr("Extraction failed"), tr("could not locate file"), extractItems[i].inPath);
                windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
                break;
            case EXTRACT_ERR_FILE_READ:
                snprintf(tempMessage, sizeof(tempMessage), "%s: %s (%s)", tr("Extraction failed"), tr("could not read file"), extractItems[i].inPath);
                windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
                break;
            case EXTRACT_ERR_FILE_WRITE:
                snprintf(tempMessage, sizeof(tempMessage), "%s: %s (%s)", tr("Extraction failed"), tr("could not write file"), extractItems[i].inPath);
                windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
                break;
            default:
                break;
            }

            if (extractStatus != EXTRACT_SUCCESS) {
                for (size_t j = 0; j < extractItemsCount; j++)
                    deletePath(extractItems[j].outPath);

                break;
            }
        }
    } else {
        char* tempDirName = strdup(getEntryTitle(e));
        safeStr(tempDirName);

        joinPath(tempExtractPath, downloadDirPath, tempDirName);

        free(tempDirName);

        snprintf(tempMessage, sizeof(tempMessage), "%s (%.124s)", tr("Do you wish to extract the content in a separate directory?"), tempExtractPath);

        if (!windowPrompt(tr("Prompt"), tempMessage, tr("Yes"), tr("No")))
            strcpy(tempExtractPath, downloadDirPath);

        snprintf(tempMessage, sizeof(tempMessage), "%s (%.124s)", tr("Extracting content..."), tempExtractPath);
        showMessage(tempMessage);

        extractStatus = extractAllZip(downloadFilePath, tempExtractPath);

        switch (extractStatus) {
        case EXTRACT_ERR_FILE_OPEN:
            snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Extraction failed"), tr("could not open file"));
            windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
            break;
        case EXTRACT_ERR_FILE_READ:
            snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Extraction failed"), tr("could not read file"));
            windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
            break;
        case EXTRACT_ERR_FILE_WRITE:
            snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Extraction failed"), tr("could not write file"));
            windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
            break;
        default:
            break;
        }
    }

    if (extractStatus == EXTRACT_SUCCESS) {
        snprintf(tempMessage, sizeof(tempMessage), "%s\n(%s)", tr("Download completed"), getEntryTitle(e));
        windowPrompt(tr("Info"), tempMessage, tr("Ok"), NULL);
    }

    deleteFile(downloadFilePath);
}

#define UPDATE_TEMP_FILENAME "tmpUpdateFile"
#define UPDATE_FILENAME "Kekatsu.nds"

// Handles the download of the update file
void downloadUpdate(void)
{
    char workDir[PATH_MAX];
    getcwd(workDir, sizeof(workDir));

    char downloadFilePath[PATH_MAX];
    joinPath(downloadFilePath, workDir, UPDATE_TEMP_FILENAME);

    DownloadStatus dlStatus = downloadGui(downloadFilePath, UPDATE_URL_APP, UPDATE_FILENAME);

    if (dlStatus != DOWNLOAD_SUCCESS) {
        if (dlStatus != DOWNLOAD_STOPPED)
            windowPrompt(tr("Error"), tr("Could not download update"), tr("Ok"), NULL);
        return;
    }

    char execPath[PATH_MAX];
    joinPath(execPath, workDir, UPDATE_FILENAME);

    renamePath(downloadFilePath, execPath);

    if (windowPrompt(tr("Info"), tr("Update downloaded successfully. Do you want to reboot now?"), tr("Yes"), tr("No")))
        switchMenu(MENU_EXIT);
}

// Handles the update check and download
void handleUpdateCheck(void)
{
    showMessage(tr("Checking for updates..."));

    char newVersion[16];
    DownloadStatus dlStatus = downloadToString(newVersion, sizeof(newVersion), UPDATE_URL_VERSION);

    if (dlStatus != DOWNLOAD_SUCCESS)
        return;

    newVersion[strcspn(newVersion, "\r\n")] = '\0';

    if (strcmp(APP_VERSION, newVersion) == 0)
        return;

    char tempMessage[128];
    snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("New version found"), newVersion);

    if (!windowPrompt(tr("Info"), tempMessage, tr("Update"), tr("Ignore")))
        return;

    downloadUpdate();
}

// Handles the loading of the given database
// Returns true if the database has been loaded, false if there was an error
bool loadDatabase(Database d)
{
    char tempMessage[128];
    snprintf(tempMessage, sizeof(tempMessage), "%s (%s)", tr("Loading database..."), getDatabaseName(d));
    showMessage(tempMessage);

    DatabaseInitStatus initStatus = initDatabase(d);
    switch (initStatus) {
    case DATABASE_INIT_ERR_DOWNLOAD:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Failed to initialize database"), tr("download failed"));
        break;
    case DATABASE_INIT_ERR_FILE_NOT_FOUND:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Failed to initialize database"), tr("file not found"));
        break;
    default:
        break;
    }

    if (initStatus != DATABASE_INIT_SUCCESS && initStatus != DATABASE_INIT_SUCCESS_CACHE) {
        windowPrompt(tr("Error"), tempMessage, tr("Ok"), NULL);
        return false;
    }

    DatabaseOpenStatus openStatus = openDatabase(d);
    switch (openStatus) {
    case DATABASE_OPEN_ERR_FILE_OPEN:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Database open failed"), tr("could not open file"));
        break;
    case DATABASE_OPEN_ERR_INVALID_VERSION:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Database open failed"), tr("invalid version"));
        break;
    case DATABASE_OPEN_ERR_INVALID_FORMAT:
        snprintf(tempMessage, sizeof(tempMessage), "%s: %s", tr("Database open failed"), tr("invalid format"));
        break;
    default:
        break;
    }

    if (openStatus != DATABASE_OPEN_SUCCESS) {
        windowPrompt(tr("Error"), tempMessage, "OK", NULL);
        return false;
    }

    if (db)
        freeDatabase(db);
    db = d;

    return true;
}

// Loads the last opened database
void loadLastOpenedDb(void)
{
    Database lastDb = getLastOpenedDatabase();
    if (!lastDb)
        return;

    if (!loadDatabase(lastDb))
        freeDatabase(lastDb);
}

// Browse menu
// Displays the search options
void browseMenu(void)
{
    menu = MENU_NONE;

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiImage brickColor1 = newGuiImage(brickColor1Bitmap, brickColor1Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    GuiImage brickColor2 = newGuiImage(brickColor2Bitmap, brickColor2Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(brickColor1, col(COLOR_PRIMARY));
    setGuiImageColorTint(brickColor2, col(COLOR_SECONDARY));
    setGuiImagePos(brickColor1, 96, 72);
    setGuiImagePos(brickColor2, 96, 72);

    char menuTitleText[32];
    snprintf(menuTitleText, sizeof(menuTitleText), "%s (%d)", tr("Browse"), (db ? getDatabaseSize(db) : 0));

    GuiText menuTitle = newGuiText(menuTitleText, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(menuTitle, 128, 178);
    setGuiTextAlignment(menuTitle, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiBox btnBg = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_BG_2));
    setGuiBoxBorder(btnBg, 2, col(COLOR_PRIMARY));
    GuiBox btnBgHover = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_PRIMARY));

    GuiText btn1Txt = newGuiText(tr("Search by title"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(btn1Txt, 12, 0);
    setGuiTextAlignment(btn1Txt, GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_MIDDLE);
    GuiButton btn1 = newGuiButton(SCREEN_WIDTH, 32);
    setGuiButtonBg(btn1, btnBg, btnBgHover);
    setGuiButtonLabel(btn1, btn1Txt);

    GuiText btn2Txt = newGuiText(tr("Browse all"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(btn2Txt, 12, 0);
    setGuiTextAlignment(btn2Txt, GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_MIDDLE);
    GuiButton btn2 = newGuiButton(SCREEN_WIDTH, 32);
    setGuiButtonBg(btn2, btnBg, btnBgHover);
    setGuiButtonLabel(btn2, btn2Txt);
    setGuiButtonPos(btn2, 0, 32);

    GuiText noDatabaseTxt = newGuiText(tr("No database loaded or initialized"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_3));
    setGuiTextPos(noDatabaseTxt, 128, 80);
    setGuiTextAlignment(noDatabaseTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(noDatabaseTxt, 244);
    setGuiTextWrap(noDatabaseTxt, true);

    addToGuiScreen(topScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, brickColor1, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, brickColor2, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, menuTitle, GUI_ELEMENT_TYPE_TEXT);

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);

    if (db && getDatabaseIsInited(db)) {
        addToGuiScreen(bottomScreen, btn1, GUI_ELEMENT_TYPE_BUTTON);
        addToGuiScreen(bottomScreen, btn2, GUI_ELEMENT_TYPE_BUTTON);
    } else {
        addToGuiScreen(bottomScreen, noDatabaseTxt, GUI_ELEMENT_TYPE_TEXT);
    }

    addNavbarToGuiScreen(bottomScreen);

    setActiveScreens(topScreen, bottomScreen);
    while (pmMainLoop() && menu == MENU_NONE) {
        guiLoop();
        drawScreens();

        if (getGuiButtonState(btn1) == GUI_BUTTON_STATE_CLICKED) {
            if (keyboardEdit(searchTitle, sizeof(searchTitle))) {
                searchPage = 1;
                switchMenu(MENU_RESULTS);
            }
            resetGuiButtonState(btn1);
        } else if (getGuiButtonState(btn2) == GUI_BUTTON_STATE_CLICKED) {
            searchTitle[0] = '\0';
            searchPage = 1;
            switchMenu(MENU_RESULTS);
        }

        navbarSwitchMenu();
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiImage(brickColor1);
    freeGuiImage(brickColor2);
    freeGuiText(menuTitle);
    freeGuiBox(btnBg);
    freeGuiBox(btnBgHover);
    freeGuiText(btn1Txt);
    freeGuiButton(btn1);
    freeGuiText(btn2Txt);
    freeGuiButton(btn2);
    freeGuiText(noDatabaseTxt);
}

#define MAX_RESULTS_PER_PAGE 50
#define MAX_RESULTS_DISPLAY 5

// Results menu
// Starts the search in the database with the set parameters and shows the results
void resultsMenu(void)
{
    menu = MENU_NONE;

    size_t resultsCount;
    Entry* results = searchDatabase(db, searchTitle, MAX_RESULTS_PER_PAGE, searchPage, &resultsCount);

    bool prevPageEnabled = (searchPage > 1);
    bool nextPageEnabled = (resultsCount == MAX_RESULTS_PER_PAGE);

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);
    setGuiScreenDpadNavigate(topScreen, 0);
    setGuiScreenDpadNavigate(bottomScreen, 0);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiImage brickColor1 = newGuiImage(brickColor1Bitmap, brickColor1Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    GuiImage brickColor2 = newGuiImage(brickColor2Bitmap, brickColor2Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(brickColor1, col(COLOR_PRIMARY));
    setGuiImageColorTint(brickColor2, col(COLOR_SECONDARY));
    setGuiImagePos(brickColor1, 96, 72);
    setGuiImagePos(brickColor2, 96, 72);

    char menuTitleText[16];
    snprintf(menuTitleText, sizeof(menuTitleText), "%s %d (%d)", tr("Page"), searchPage, resultsCount);
    GuiText menuTitle = newGuiText(menuTitleText, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(menuTitle, 128, 178);
    setGuiTextAlignment(menuTitle, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText prevPageTxt = newGuiText(tr("Previous"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT));
    GuiText nextPageTxt = newGuiText(tr("Next"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT));
    setGuiTextPos(prevPageTxt, 26, 180);
    setGuiTextPos(nextPageTxt, 230, 180);
    setGuiTextAlignment(prevPageTxt, GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextAlignment(nextPageTxt, GUI_TEXT_H_ALIGN_RIGHT, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiImage lButtonIcon = newGuiImage(lButtonBitmap, lButtonPal, 16, 10, 16, 16, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    GuiImage rButtonIcon = newGuiImage(rButtonBitmap, rButtonPal, 16, 10, 16, 16, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImagePos(lButtonIcon, 6, 174);
    setGuiImagePos(rButtonIcon, 234, 174);

    GuiBox resultsBtnBg = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_BG_2));
    setGuiBoxBorder(resultsBtnBg, 2, col(COLOR_PRIMARY));
    GuiBox resultsBtnBgHover = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_PRIMARY));

    GuiText resultsBtnTxt[resultsCount];
    GuiText resultsBtnTxtPlatform[resultsCount];
    GuiText resultsBtnTxtRegion[resultsCount];
    GuiButton resultsBtn[resultsCount];

    for (size_t i = 0; i < resultsCount; i++) {
        resultsBtnTxt[i] = newGuiText(getEntryTitle(results[i]), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
        setGuiTextPos(resultsBtnTxt[i], 12, 0);
        setGuiTextAlignment(resultsBtnTxt[i], GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxWidth(resultsBtnTxt[i], 186);
        setGuiTextMaxHeight(resultsBtnTxt[i], 32);
        setGuiTextWrap(resultsBtnTxt[i], true);

        resultsBtnTxtPlatform[i] = newGuiText(getEntryPlatform(results[i]), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
        setGuiTextAlignment(resultsBtnTxtPlatform[i], GUI_TEXT_H_ALIGN_RIGHT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxWidth(resultsBtnTxtPlatform[i], 48);

        resultsBtnTxtRegion[i] = newGuiText(getEntryRegion(results[i]), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
        setGuiTextAlignment(resultsBtnTxtRegion[i], GUI_TEXT_H_ALIGN_RIGHT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxWidth(resultsBtnTxtRegion[i], 48);

        resultsBtn[i] = newGuiButton(SCREEN_WIDTH, 32);
        setGuiButtonBg(resultsBtn[i], resultsBtnBg, resultsBtnBgHover);
        setGuiButtonLabel(resultsBtn[i], resultsBtnTxt[i]);
    }

    addToGuiScreen(topScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, brickColor1, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, brickColor2, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, menuTitle, GUI_ELEMENT_TYPE_TEXT);

    if (prevPageEnabled) {
        addToGuiScreen(topScreen, prevPageTxt, GUI_ELEMENT_TYPE_TEXT);
        addToGuiScreen(topScreen, lButtonIcon, GUI_ELEMENT_TYPE_IMAGE);
    }

    if (nextPageEnabled) {
        addToGuiScreen(topScreen, nextPageTxt, GUI_ELEMENT_TYPE_TEXT);
        addToGuiScreen(topScreen, rButtonIcon, GUI_ELEMENT_TYPE_IMAGE);
    }

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);

    addNavbarToGuiScreen(bottomScreen);

    bool firstRun = true;

    size_t scrollOffset = 0;
    size_t selectOffset = 0;

    bool selectUpAction = false;
    bool selectDownAction = false;
    bool scrollUpAction = false;
    bool scrollDownAction = false;
    bool scrollUpFastAction = false;
    bool scrollDownFastAction = false;

    bool clickAction = false;

    bool nextPageAction = false;
    bool prevPageAction = false;

    bool selectAction = false;
    bool scrollAction = false;
    bool switchPageAction = false;

    size_t displayedResultsCount = min(MAX_RESULTS_DISPLAY, resultsCount);
    if (resultsCount > 0)
        setGuiButtonState(resultsBtn[0], GUI_BUTTON_STATE_SELECTED);

    setActiveScreens(topScreen, bottomScreen);
    while (pmMainLoop() && menu == MENU_NONE) {
        guiLoop();

        if (resultsCount > 0) {
            selectUpAction = (pressed & KEY_UP);
            selectDownAction = (pressed & KEY_DOWN);
            scrollUpAction = (selectUpAction && getGuiButtonState(resultsBtn[scrollOffset]) == GUI_BUTTON_STATE_SELECTED); // Up is pressed and highest result is selected
            scrollDownAction = (selectDownAction && getGuiButtonState(resultsBtn[scrollOffset + displayedResultsCount - 1]) == GUI_BUTTON_STATE_SELECTED); // Down is pressed and lowest result is selected
            scrollUpFastAction = (pressed & KEY_LEFT);
            scrollDownFastAction = (pressed & KEY_RIGHT);
            clickAction = (pressed & KEY_A);
            nextPageAction = (pressed & KEY_R);
            prevPageAction = (pressed & KEY_L);

            selectAction = selectUpAction || selectDownAction;
            scrollAction = scrollUpAction || scrollDownAction || scrollUpFastAction || scrollDownFastAction;
            switchPageAction = nextPageAction || prevPageAction;

            if (scrollAction || firstRun) {
                resetGuiButtonState(resultsBtn[scrollOffset + selectOffset]);

                if (!firstRun) {
                    // Remove all displayed results
                    for (size_t i = 0; i < displayedResultsCount; i++) {
                        removeFromGuiScreen(bottomScreen, resultsBtn[i + scrollOffset]);
                        removeFromGuiScreen(bottomScreen, resultsBtnTxtPlatform[i + scrollOffset]);
                        removeFromGuiScreen(bottomScreen, resultsBtnTxtRegion[i + scrollOffset]);
                    }
                } else {
                    firstRun = false;
                }

                if (scrollUpAction) {
                    if (scrollOffset > 0)
                        scrollOffset--;

                    setGuiButtonState(resultsBtn[scrollOffset], GUI_BUTTON_STATE_SELECTED);
                    selectOffset = 0;
                } else if (scrollDownAction) {
                    if (scrollOffset < resultsCount - 5)
                        scrollOffset++;

                    setGuiButtonState(resultsBtn[scrollOffset + displayedResultsCount - 1], GUI_BUTTON_STATE_SELECTED);
                    selectOffset = displayedResultsCount - 1;
                } else if (scrollUpFastAction) {
                    scrollOffset -= min(MAX_RESULTS_DISPLAY, scrollOffset);

                    setGuiButtonState(resultsBtn[scrollOffset], GUI_BUTTON_STATE_SELECTED);
                    selectOffset = 0;
                } else if (scrollDownFastAction) {
                    size_t maxPossibleStep = resultsCount - displayedResultsCount - scrollOffset;
                    if (maxPossibleStep > 0)
                        scrollOffset += min(MAX_RESULTS_DISPLAY, maxPossibleStep);

                    setGuiButtonState(resultsBtn[scrollOffset], GUI_BUTTON_STATE_SELECTED);
                    selectOffset = 0;
                }

                // Add displayed results according to scroll offset
                for (size_t i = 0; i < displayedResultsCount; i++) {
                    setGuiButtonPos(resultsBtn[i + scrollOffset], 0, i * 32);
                    setGuiTextPos(resultsBtnTxtPlatform[i + scrollOffset], 246, i * 32 + 9);
                    setGuiTextPos(resultsBtnTxtRegion[i + scrollOffset], 246, i * 32 + 23);

                    addToGuiScreen(bottomScreen, resultsBtn[i + scrollOffset], GUI_ELEMENT_TYPE_BUTTON);
                    addToGuiScreen(bottomScreen, resultsBtnTxtPlatform[i + scrollOffset], GUI_ELEMENT_TYPE_TEXT);
                    addToGuiScreen(bottomScreen, resultsBtnTxtRegion[i + scrollOffset], GUI_ELEMENT_TYPE_TEXT);
                }
            } else if (selectAction) {
                resetGuiButtonState(resultsBtn[scrollOffset + selectOffset]);

                if (selectUpAction && selectOffset > 0)
                    selectOffset--;
                else if (selectDownAction && selectOffset < MAX_RESULTS_PER_PAGE - 1)
                    selectOffset++;

                setGuiButtonState(resultsBtn[scrollOffset + selectOffset], GUI_BUTTON_STATE_SELECTED);
            } else if (clickAction) {
                setGuiButtonState(resultsBtn[scrollOffset + selectOffset], GUI_BUTTON_STATE_CLICKED);
            } else if (switchPageAction) {
                // Set page and refresh menu
                if (nextPageEnabled && nextPageAction) {
                    searchPage++;
                    switchMenu(MENU_RESULTS);
                } else if (prevPageEnabled && prevPageAction) {
                    searchPage--;
                    switchMenu(MENU_RESULTS);
                }
            } else {
                // Check for clicked result, set selected entry and switch to entry menu
                for (size_t i = 0; i < displayedResultsCount; i++) {
                    if (getGuiButtonState(resultsBtn[i + scrollOffset]) != GUI_BUTTON_STATE_CLICKED)
                        continue;

                    if (selectedEntry)
                        freeEntry(selectedEntry);
                    selectedEntry = cloneEntry(results[i + scrollOffset]);
                    switchMenu(MENU_ENTRY);
                }
            }
        }

        drawScreens();

        navbarSwitchMenu();
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiImage(brickColor1);
    freeGuiImage(brickColor2);
    freeGuiText(menuTitle);
    freeGuiText(prevPageTxt);
    freeGuiText(nextPageTxt);
    freeGuiImage(lButtonIcon);
    freeGuiImage(rButtonIcon);
    freeGuiBox(resultsBtnBg);
    freeGuiBox(resultsBtnBgHover);

    for (size_t i = 0; i < resultsCount; i++) {
        freeGuiText(resultsBtnTxt[i]);
        freeGuiText(resultsBtnTxtPlatform[i]);
        freeGuiText(resultsBtnTxtRegion[i]);
        freeGuiButton(resultsBtn[i]);
        freeEntry(results[i]);
    }
}

// Entry menu
// Displays the information of the selected entry
void entryMenu(void)
{
    menu = MENU_NONE;

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox topBg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG_2));
    GuiBox bottomBg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiImage brickColor1 = NULL;
    GuiImage brickColor2 = NULL;

    GuiText entryTitle = newGuiText(getEntryTitle(selectedEntry), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(entryTitle, 128, 20);
    setGuiTextAlignment(entryTitle, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(entryTitle, 246);
    setGuiTextWrap(entryTitle, true);
    setGuiTextMaxHeight(entryTitle, 40);

    GuiText entryPlatformLabel = newGuiText(tr("Platform"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT));
    setGuiTextPos(entryPlatformLabel, 80, 46);
    setGuiTextAlignment(entryPlatformLabel, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText entryPlatform = newGuiText(getEntryPlatform(selectedEntry), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    setGuiTextPos(entryPlatform, 176, 46);
    setGuiTextAlignment(entryPlatform, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(entryPlatform, 96);

    GuiText entryRegionLabel = newGuiText(tr("Region"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT));
    setGuiTextPos(entryRegionLabel, 80, 64);
    setGuiTextAlignment(entryRegionLabel, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText entryRegion = newGuiText(getEntryRegion(selectedEntry), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    setGuiTextPos(entryRegion, 176, 64);
    setGuiTextAlignment(entryRegion, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(entryRegion, 96);

    GuiText entryAuthorLabel = newGuiText(tr("Author"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT));
    setGuiTextPos(entryAuthorLabel, 80, 82);
    setGuiTextAlignment(entryAuthorLabel, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText entryAuthor = newGuiText(getEntryAuthor(selectedEntry), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    setGuiTextPos(entryAuthor, 176, 82);
    setGuiTextAlignment(entryAuthor, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(entryAuthor, 96);

    GuiText entryVersionLabel = newGuiText(tr("Version"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT));
    setGuiTextPos(entryVersionLabel, 80, 100);
    setGuiTextAlignment(entryVersionLabel, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText entryVersion = newGuiText(getEntryVersion(selectedEntry), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    setGuiTextPos(entryVersion, 176, 100);
    setGuiTextAlignment(entryVersion, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(entryVersion, 96);

    char downloadBtnTxtText[32];
    u64 entrySize = getEntrySize(selectedEntry);

    if (entrySize) {
        char sizeText[16];
        humanizeSize(sizeText, sizeof(sizeText), getEntrySize(selectedEntry));
        snprintf(downloadBtnTxtText, sizeof(downloadBtnTxtText), "%s (%s)", tr("Download"), sizeText);
    } else {
        snprintf(downloadBtnTxtText, sizeof(downloadBtnTxtText), "%s", tr("Download"));
    }

    GuiText downloadBtnTxt = newGuiText(downloadBtnTxtText, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextAlignment(downloadBtnTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiBox downloadBtnBg = newGuiBox(getGuiTextWidth(downloadBtnTxt) + 20, 34, col(COLOR_BG_2));
    setGuiBoxBorder(downloadBtnBg, 2, col(COLOR_PRIMARY));
    GuiBox downloadBtnBgHover = newGuiBox(getGuiTextWidth(downloadBtnTxt) + 20, 34, col(COLOR_PRIMARY));

    GuiButton downloadBtn = newGuiButton(getGuiTextWidth(downloadBtnTxt) + 20, 34);
    setGuiButtonPos(downloadBtn, SCREEN_WIDTH / 2 - (getGuiTextWidth(downloadBtnTxt) + 20) / 2, 114);
    setGuiButtonBg(downloadBtn, downloadBtnBg, downloadBtnBgHover);
    setGuiButtonLabel(downloadBtn, downloadBtnTxt);

    GuiImage boxart = loadBoxart(selectedEntry);

    addToGuiScreen(topScreen, topBg, GUI_ELEMENT_TYPE_BOX);

    if (boxart) {
        setGuiImagePos(boxart, 128, 96);
        setGuiImageAlign(boxart, GUI_IMAGE_H_ALIGN_CENTER, GUI_IMAGE_V_ALIGN_MIDDLE);

        addToGuiScreen(topScreen, boxart, GUI_ELEMENT_TYPE_IMAGE);
    } else {
        brickColor1 = newGuiImage(brickColor1Bitmap, brickColor1Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
        brickColor2 = newGuiImage(brickColor2Bitmap, brickColor2Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
        setGuiImageColorTint(brickColor1, col(COLOR_PRIMARY));
        setGuiImageColorTint(brickColor2, col(COLOR_SECONDARY));
        setGuiImagePos(brickColor1, 96, 72);
        setGuiImagePos(brickColor2, 96, 72);

        addToGuiScreen(topScreen, brickColor1, GUI_ELEMENT_TYPE_IMAGE);
        addToGuiScreen(topScreen, brickColor2, GUI_ELEMENT_TYPE_IMAGE);
    }

    addToGuiScreen(bottomScreen, bottomBg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, entryTitle, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryPlatformLabel, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryPlatform, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryRegionLabel, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryRegion, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryAuthorLabel, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryAuthor, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryVersionLabel, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, entryVersion, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, downloadBtn, GUI_ELEMENT_TYPE_BUTTON);

    addNavbarToGuiScreen(bottomScreen);

    setActiveScreens(topScreen, bottomScreen);

    while (pmMainLoop() && menu == MENU_NONE) {
        guiLoop();
        drawScreens();

        if (getGuiButtonState(downloadBtn) == GUI_BUTTON_STATE_CLICKED)
            switchMenu(MENU_DOWNLOAD);

        navbarSwitchMenu();
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(topBg);
    freeGuiBox(bottomBg);
    freeGuiText(entryTitle);
    freeGuiText(entryPlatformLabel);
    freeGuiText(entryPlatform);
    freeGuiText(entryRegionLabel);
    freeGuiText(entryRegion);
    freeGuiText(entryAuthorLabel);
    freeGuiText(entryAuthor);
    freeGuiText(entryVersionLabel);
    freeGuiText(entryVersion);
    freeGuiText(downloadBtnTxt);
    freeGuiBox(downloadBtnBg);
    freeGuiBox(downloadBtnBgHover);
    freeGuiButton(downloadBtn);

    if (boxart) {
        freeGuiImage(boxart);
    } else {
        freeGuiImage(brickColor1);
        freeGuiImage(brickColor2);
    }
}

// Download menu
// Starts the download of the selected entry
void downloadMenu(void)
{
    downloadEntry(selectedEntry);

    switchMenu(MENU_BROWSE);
}

#define MAX_DATABASES_DISPLAY 8

// Databases menu
// Displays the list of available databases
void databasesMenu(void)
{
    menu = MENU_NONE;

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiImage brickColor1 = newGuiImage(brickColor1Bitmap, brickColor1Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    GuiImage brickColor2 = newGuiImage(brickColor2Bitmap, brickColor2Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(brickColor1, col(COLOR_PRIMARY));
    setGuiImageColorTint(brickColor2, col(COLOR_SECONDARY));
    setGuiImagePos(brickColor1, 96, 72);
    setGuiImagePos(brickColor2, 96, 72);

    GuiText menuTitle = newGuiText(tr("Databases"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(menuTitle, 128, 178);
    setGuiTextAlignment(menuTitle, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText noDatabasesTxt = newGuiText(tr("No databases found"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_3));
    setGuiTextPos(noDatabasesTxt, 128, 80);
    setGuiTextAlignment(noDatabasesTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);
    setGuiTextMaxWidth(noDatabasesTxt, 244);
    setGuiTextWrap(noDatabasesTxt, true);

    GuiBox btnBg = newGuiBox(SCREEN_WIDTH, 20, col(COLOR_BG_2));
    setGuiBoxBorder(btnBg, 2, col(COLOR_PRIMARY));

    GuiBox btnBgUsed = newGuiBox(SCREEN_WIDTH, 20, col(COLOR_SECONDARY));
    setGuiBoxBorder(btnBgUsed, 2, col(COLOR_PRIMARY));

    GuiBox btnBgHover = newGuiBox(SCREEN_WIDTH, 20, col(COLOR_PRIMARY));

    addToGuiScreen(topScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, brickColor1, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, brickColor2, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, menuTitle, GUI_ELEMENT_TYPE_TEXT);

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);

    // Get database list and create screen elements
    size_t dbCount;
    Database* dbList = getDatabaseList(&dbCount);

    size_t displayedDatabasesCount = min(dbCount, MAX_DATABASES_DISPLAY);

    GuiText dbBtnTxtNames[dbCount];
    GuiText dbBtnTxtValues[dbCount];
    GuiButton dbBtns[dbCount];

    for (size_t i = 0; i < displayedDatabasesCount; i++) {
        dbBtnTxtNames[i] = newGuiText(getDatabaseName(dbList[i]), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
        setGuiTextPos(dbBtnTxtNames[i], 10, 0);
        setGuiTextAlignment(dbBtnTxtNames[i], GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxWidth(dbBtnTxtNames[i], 236);

        dbBtnTxtValues[i] = newGuiText(getDatabaseValue(dbList[i]), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_3));
        setGuiTextPos(dbBtnTxtValues[i], 246, i * 20 + 10);
        setGuiTextAlignment(dbBtnTxtValues[i], GUI_TEXT_H_ALIGN_RIGHT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxWidth(dbBtnTxtValues[i], 236 - getGuiTextWidth(dbBtnTxtNames[i]));

        dbBtns[i] = newGuiButton(SCREEN_WIDTH, 20);

        // Use different bg color if database is being used
        if (db && strcmp(getDatabaseValue(db), getDatabaseValue(dbList[i])) == 0)
            setGuiButtonBg(dbBtns[i], btnBgUsed, btnBgHover);
        else
            setGuiButtonBg(dbBtns[i], btnBg, btnBgHover);

        setGuiButtonLabel(dbBtns[i], dbBtnTxtNames[i]);
        setGuiButtonPos(dbBtns[i], 0, i * 20);

        addToGuiScreen(bottomScreen, dbBtns[i], GUI_ELEMENT_TYPE_BUTTON);
        addToGuiScreen(bottomScreen, dbBtnTxtValues[i], GUI_ELEMENT_TYPE_TEXT);
    }

    if (dbCount == 0)
        addToGuiScreen(bottomScreen, noDatabasesTxt, GUI_ELEMENT_TYPE_TEXT);

    addNavbarToGuiScreen(bottomScreen);

    setActiveScreens(topScreen, bottomScreen);
    while (pmMainLoop() && menu == MENU_NONE) {
        guiLoop();
        drawScreens();

        for (size_t i = 0; i < displayedDatabasesCount; i++) {
            if (getGuiButtonState(dbBtns[i]) != GUI_BUTTON_STATE_CLICKED)
                continue;

            Database d = dbList[i];
            loadDatabase(d);

            switchMenu(MENU_DATABASES);
        }

        navbarSwitchMenu();
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiImage(brickColor1);
    freeGuiImage(brickColor2);
    freeGuiText(menuTitle);
    freeGuiText(noDatabasesTxt);
    freeGuiBox(btnBg);
    freeGuiBox(btnBgUsed);
    freeGuiBox(btnBgHover);

    for (size_t i = 0; i < displayedDatabasesCount; i++) {
        freeGuiText(dbBtnTxtNames[i]);
        freeGuiText(dbBtnTxtValues[i]);
        freeGuiButton(dbBtns[i]);
    }

    for (size_t i = 0; i < dbCount; i++) {
        if (dbList[i] != db)
            freeDatabase(dbList[i]);
    }
}

// Settings menu
// Displays the app settings
void settingsMenu(void)
{
    menu = MENU_NONE;

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiImage brickColor1 = newGuiImage(brickColor1Bitmap, brickColor1Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    GuiImage brickColor2 = newGuiImage(brickColor2Bitmap, brickColor2Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(brickColor1, col(COLOR_PRIMARY));
    setGuiImageColorTint(brickColor2, col(COLOR_SECONDARY));
    setGuiImagePos(brickColor1, 96, 72);
    setGuiImagePos(brickColor2, 96, 72);

    GuiText menuTitle = newGuiText(tr("Settings"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(menuTitle, 128, 178);
    setGuiTextAlignment(menuTitle, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiBox btnBg = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_BG_2));
    setGuiBoxBorder(btnBg, 2, col(COLOR_PRIMARY));

    GuiBox btnBgHover = newGuiBox(SCREEN_WIDTH, 32, col(COLOR_PRIMARY));

    GuiText settingBtnTxt[SETTINGS_COUNT];
    GuiButton settingBtn[SETTINGS_COUNT];
    GuiText settingValueTxt[SETTINGS_COUNT];

    char colorSchemeStr[4];
    snprintf(colorSchemeStr, sizeof(colorSchemeStr), "%d", settings.colorScheme + 1);

    settingBtnTxt[0] = newGuiText(tr("Downloads directory"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    settingValueTxt[0] = newGuiText(settings.dlPath, GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_3));
    setGuiTextMaxWidth(settingBtnTxt[0], 122);

    settingBtnTxt[1] = newGuiText(tr("Use platform-specific directories (E.g. nds, gba)"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    settingValueTxt[1] = newGuiText(tr(dlUseDirsStr(settings.dlUseDirs)), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_3));
    setGuiTextMaxWidth(settingBtnTxt[1], 168);

    settingBtnTxt[2] = newGuiText(tr("Color scheme"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    settingValueTxt[2] = newGuiText(colorSchemeStr, GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_3));
    setGuiTextMaxWidth(settingBtnTxt[2], 168);

    settingBtnTxt[3] = newGuiText(tr("Check for updates on start"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    settingValueTxt[3] = newGuiText(tr(checkUpdateOnStartStr(settings.checkUpdateOnStart)), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_3));
    setGuiTextMaxWidth(settingBtnTxt[3], 168);

    settingBtnTxt[4] = newGuiText(tr("Language"), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    settingValueTxt[4] = newGuiText(tr(langStr(settings.lang)), GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_3));
    setGuiTextMaxWidth(settingBtnTxt[4], 168);

    addToGuiScreen(topScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, brickColor1, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, brickColor2, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, menuTitle, GUI_ELEMENT_TYPE_TEXT);

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);

    for (size_t i = 0; i < SETTINGS_COUNT; i++) {
        setGuiTextPos(settingBtnTxt[i], 12, 0);
        setGuiTextAlignment(settingBtnTxt[i], GUI_TEXT_H_ALIGN_LEFT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxHeight(settingBtnTxt[i], 32);
        setGuiTextWrap(settingBtnTxt[i], true);

        setGuiTextPos(settingValueTxt[i], 244, i * 32 + 16);
        setGuiTextAlignment(settingValueTxt[i], GUI_TEXT_H_ALIGN_RIGHT, GUI_TEXT_V_ALIGN_MIDDLE);
        setGuiTextMaxWidth(settingValueTxt[i], 232 - getGuiTextWidth(settingBtnTxt[i]));
        setGuiTextMaxHeight(settingValueTxt[i], 32);
        setGuiTextWrap(settingValueTxt[i], true);

        settingBtn[i] = newGuiButton(SCREEN_WIDTH, 32);
        setGuiButtonBg(settingBtn[i], btnBg, btnBgHover);
        setGuiButtonLabel(settingBtn[i], settingBtnTxt[i]);
        setGuiButtonPos(settingBtn[i], 0, i * 32);

        addToGuiScreen(bottomScreen, settingBtn[i], GUI_ELEMENT_TYPE_BUTTON);
        addToGuiScreen(bottomScreen, settingValueTxt[i], GUI_ELEMENT_TYPE_TEXT);
    }

    addNavbarToGuiScreen(bottomScreen);

    setActiveScreens(topScreen, bottomScreen);
    while (pmMainLoop() && menu == MENU_NONE) {
        guiLoop();
        drawScreens();

        if (getGuiButtonState(settingBtn[0]) == GUI_BUTTON_STATE_CLICKED) {
            char tempDlPath[PATH_MAX];
            strcpy(tempDlPath, settings.dlPath);

            if (keyboardEdit(tempDlPath, sizeof(tempDlPath))) {
                bool save = true;

                if (tempDlPath[0] == '\0') {
                    windowPrompt(tr("Error"), tr("Path can not be empty"), tr("Ok"), NULL);
                    save = false;
                } else if (!dirExists(tempDlPath)) {
                    if (windowPrompt(tr("Error"), tr("Directory does not exist. Do you want to create it?"), tr("Ok"), tr("Cancel"))) {
                        if (!createDirR(tempDlPath)) {
                            windowPrompt(tr("Error"), tr("Failed to create directory"), tr("Ok"), NULL);
                            save = false;
                        }
                    } else {
                        save = false;
                    }
                }

                if (save) {
                    strcpy(settings.dlPath, tempDlPath);
                    saveSettings();
                }
            }

            setGuiTextText(settingValueTxt[0], settings.dlPath);

            resetGuiButtonState(settingBtn[0]);
        } else if (getGuiButtonState(settingBtn[1]) == GUI_BUTTON_STATE_CLICKED) {
            settings.dlUseDirs = settings.dlUseDirs ? false : true;

            setGuiTextText(settingValueTxt[1], tr(dlUseDirsStr(settings.dlUseDirs)));

            saveSettings();
            resetGuiButtonState(settingBtn[1]);
        } else if (getGuiButtonState(settingBtn[2]) == GUI_BUTTON_STATE_CLICKED) {
            settings.colorScheme++;

            if (settings.colorScheme == COLOR_SCHEMES_COUNT)
                settings.colorScheme = 0;

            snprintf(colorSchemeStr, sizeof(colorSchemeStr), "%d", settings.colorScheme + 1);
            setGuiTextText(settingValueTxt[2], colorSchemeStr);

            freeNavbar();
            initNavbar();
            switchMenu(MENU_SETTINGS);

            saveSettings();
            resetGuiButtonState(settingBtn[2]);
        } else if (getGuiButtonState(settingBtn[3]) == GUI_BUTTON_STATE_CLICKED) {
            settings.checkUpdateOnStart = settings.checkUpdateOnStart ? false : true;

            setGuiTextText(settingValueTxt[3], tr(checkUpdateOnStartStr(settings.checkUpdateOnStart)));

            saveSettings();
            resetGuiButtonState(settingBtn[3]);
        } else if (getGuiButtonState(settingBtn[4]) == GUI_BUTTON_STATE_CLICKED) {
            settings.lang++;

            if (settings.lang == LANGS_COUNT)
                settings.lang = 0;

            setGuiTextText(settingValueTxt[4], tr(langStr(settings.lang)));

            loadLanguage(settings.lang);
            switchMenu(MENU_SETTINGS);

            saveSettings();
            resetGuiButtonState(settingBtn[4]);
        }

        navbarSwitchMenu();
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiImage(brickColor1);
    freeGuiImage(brickColor2);
    freeGuiText(menuTitle);
    freeGuiBox(btnBg);
    freeGuiBox(btnBgHover);

    for (size_t i = 0; i < SETTINGS_COUNT; i++) {
        freeGuiText(settingBtnTxt[i]);
        freeGuiText(settingValueTxt[i]);
        freeGuiButton(settingBtn[i]);
    }
}

// Info menu
// Displays information about Kekatsu
void infoMenu(void)
{
    menu = MENU_NONE;

    GuiScreen topScreen = newGuiScreen(GUI_SCREEN_LCD_TOP);
    GuiScreen bottomScreen = newGuiScreen(GUI_SCREEN_LCD_BOTTOM);

    GuiBox bg = newGuiBox(SCREEN_WIDTH, SCREEN_HEIGHT, col(COLOR_BG));

    GuiImage brickColor1 = newGuiImage(brickColor1Bitmap, brickColor1Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    GuiImage brickColor2 = newGuiImage(brickColor2Bitmap, brickColor2Pal, 64, 48, 64, 64, 0, 0, GUI_IMAGE_TEXTURE_TYPE_RGB256);
    setGuiImageColorTint(brickColor1, col(COLOR_PRIMARY));
    setGuiImageColorTint(brickColor2, col(COLOR_SECONDARY));
    setGuiImagePos(brickColor1, 96, 72);
    setGuiImagePos(brickColor2, 96, 72);

    GuiText menuTitle = newGuiText(tr("Info"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(menuTitle, 128, 178);
    setGuiTextAlignment(menuTitle, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText appNameTxt = newGuiText(APP_NAME, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT));
    setGuiTextPos(appNameTxt, 128, 22);
    setGuiTextAlignment(appNameTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    char appVersionText[16];
    snprintf(appVersionText, sizeof(appVersionText), "%s %s", tr("Version"), APP_VERSION);

    GuiText appVersionTxt = newGuiText(appVersionText, GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextPos(appVersionTxt, 128, 43);
    setGuiTextAlignment(appVersionTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText subtitle1Txt = newGuiText("by Cavv", GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    setGuiTextPos(subtitle1Txt, 128, 72);
    setGuiTextAlignment(subtitle1Txt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText subtitle2Txt = newGuiText("https://github.com/cavv-dev/Kekatsu-DS", GUI_TEXT_SIZE_SMALL, col(COLOR_TEXT_2));
    setGuiTextPos(subtitle2Txt, 128, 87);
    setGuiTextAlignment(subtitle2Txt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    GuiText checkUpdateBtnTxt = newGuiText(tr("Check for updates"), GUI_TEXT_SIZE_MEDIUM, col(COLOR_TEXT_2));
    setGuiTextAlignment(checkUpdateBtnTxt, GUI_TEXT_H_ALIGN_CENTER, GUI_TEXT_V_ALIGN_MIDDLE);

    size_t checkUpdateBtnWidth = getGuiTextWidth(checkUpdateBtnTxt) + 20;

    GuiBox checkUpdateBtnBg = newGuiBox(checkUpdateBtnWidth, 34, col(COLOR_BG_2));
    setGuiBoxBorder(checkUpdateBtnBg, 2, col(COLOR_PRIMARY));
    GuiBox checkUpdateBtnBgHover = newGuiBox(checkUpdateBtnWidth, 32, col(COLOR_PRIMARY));

    GuiButton checkUpdateBtn = newGuiButton(checkUpdateBtnWidth, 34);
    setGuiButtonBg(checkUpdateBtn, checkUpdateBtnBg, checkUpdateBtnBgHover);
    setGuiButtonLabel(checkUpdateBtn, checkUpdateBtnTxt);
    setGuiButtonPos(checkUpdateBtn, 128 - checkUpdateBtnWidth / 2, 106);

    addToGuiScreen(topScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(topScreen, brickColor1, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, brickColor2, GUI_ELEMENT_TYPE_IMAGE);
    addToGuiScreen(topScreen, menuTitle, GUI_ELEMENT_TYPE_TEXT);

    addToGuiScreen(bottomScreen, bg, GUI_ELEMENT_TYPE_BOX);
    addToGuiScreen(bottomScreen, appNameTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, appVersionTxt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, subtitle1Txt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, subtitle2Txt, GUI_ELEMENT_TYPE_TEXT);
    addToGuiScreen(bottomScreen, checkUpdateBtn, GUI_ELEMENT_TYPE_BUTTON);

    addNavbarToGuiScreen(bottomScreen);

    setActiveScreens(topScreen, bottomScreen);
    while (pmMainLoop() && menu == MENU_NONE) {
        guiLoop();
        drawScreens();

        if (getGuiButtonState(checkUpdateBtn) == GUI_BUTTON_STATE_CLICKED) {
            handleUpdateCheck();
            resetGuiButtonState(checkUpdateBtn);
        }

        navbarSwitchMenu();
    }

    freeGuiScreen(topScreen);
    freeGuiScreen(bottomScreen);
    freeGuiBox(bg);
    freeGuiImage(brickColor1);
    freeGuiImage(brickColor2);
    freeGuiText(menuTitle);
    freeGuiText(appNameTxt);
    freeGuiText(appVersionTxt);
    freeGuiText(subtitle1Txt);
    freeGuiText(subtitle2Txt);
    freeGuiText(checkUpdateBtnTxt);
    freeGuiBox(checkUpdateBtnBg);
    freeGuiBox(checkUpdateBtnBgHover);
    freeGuiButton(checkUpdateBtn);
}

// Initializes the GUI elements and starts the menu system
void menuBegin(MenuEnum startingMenu)
{
    drawBlank();

    loadLanguage(settings.lang);
    initColorSchemes();
    initNavbar();

    switchMenu(startingMenu);

    loadLastOpenedDb();

    if (settings.checkUpdateOnStart)
        handleUpdateCheck();

    while (menu != MENU_EXIT) {
        switch (menu) {
        case MENU_BROWSE:
            browseMenu();
            break;
        case MENU_RESULTS:
            resultsMenu();
            break;
        case MENU_ENTRY:
            entryMenu();
            break;
        case MENU_DOWNLOAD:
            downloadMenu();
            break;
        case MENU_DATABASES:
            databasesMenu();
            break;
        case MENU_SETTINGS:
            settingsMenu();
            break;
        case MENU_INFO:
            infoMenu();
            break;
        default:
            break;
        }
    }

    drawBlank();
}

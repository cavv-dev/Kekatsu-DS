#include "main.h"

#include "config.h"
#include "filesystem.h"
#include "gui/text.h"
#include "gui/video.h"
#include "menu.h"
#include "networking.h"
#include "settings.h"
#include "utils/filesystem.h"
#include <fat.h>
#include <nds.h>

Database db = NULL;

static void sleepSeconds(u32 seconds)
{
    for (u32 i = 0; i < seconds * 60; i++)
        cothread_yield_irq(IRQ_VBLANK);
}

static void handleInit(bool result, const char* infoMessage, const char* errorMessage, u32 sleepTime, bool exitOnError)
{
    printf(infoMessage);

    if (result) {
        printf("Ok!\n");
        return;
    }

    printf(errorMessage);
    sleepSeconds(sleepTime);

    if (exitOnError)
        exit(1);
}

int main(int argc, char** argv)
{
    consoleDemoInit();

    printf("\n\t\t\t" APP_NAME " " APP_VERSION "\n\n");
    printf("Initializing\n\n");

    handleInit(initFat(), "Filesystem...", "\n\nFailed to initialize filesystem\n", 5, true);
    handleInit(initNetworking() == NETWORKING_INIT_SUCCESS, "Networking...", "\n\nFailed to initialize networking:\nwifi connection failed\n", 5, true);
    handleInit((createDir(APPDATA_DIR) && createDir(CACHE_DIR)), "Directories...", "\n\nFailed to initialize directories\n", 5, true);
    handleInit((defaultSettings() && loadSettings()), "Settings...", "\n\nLoaded default settings\n", 3, false);

    initGuiVideo();
    initGuiFont();

    menuBegin(MENU_BROWSE);

    return 0;
}

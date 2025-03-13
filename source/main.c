#include "main.h"

#include "config.h"
#include "gui/text.h"
#include "gui/video.h"
#include "menu.h"
#include "networking.h"
#include "settings.h"
#include "utils/filesystem.h"
#include <fat.h>
#include <nds.h>
#include <unistd.h>

Database db = NULL;

void handleInit(bool result, const char* infoMessage, const char* errorMessage, u8 sleepTime, bool exitOnError)
{
    iprintf(infoMessage);

    if (result) {
        iprintf("Ok!\n");
        return;
    }

    iprintf(errorMessage);
    sleep(sleepTime);

    if (exitOnError)
        exit(1);
}

int main(void)
{
    consoleDemoInit();

    iprintf("\n\t\t\t" APP_NAME " " APP_VERSION "\n\n");
    iprintf("Initializing\n\n");

    handleInit(fatInitDefault(), "Filesystem...", "\n\nFailed to initialize filesystem\n", 5, true);
    handleInit(initNetworking() == NETWORKING_INIT_SUCCESS, "Networking...", "\n\nFailed to initialize networking:\nwifi connection failed\n", 5, true);
    handleInit((createDir(APPDATA_DIR) && createDir(CACHE_DIR)), "Directories...", "\n\nFailed to initialize directories\n", 5, true);
    handleInit((defaultSettings() && loadSettings()), "Settings...", "\n\nLoaded default settings\n", 3, false);

    initGuiVideo();
    initGuiFont();

    menuBegin(MENU_BROWSE);

    return 0;
}

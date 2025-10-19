#include "filesystem.h"

#include <fat.h>
#include <nds.h>
#include <sys/syslimits.h>
#include <unistd.h>

char romDir[PATH_MAX];

void loadRomDir(void)
{
    // Check if argv has been provided by the loader
    if (__system_argv->argvMagic == ARGV_MAGIC && __system_argv->argc >= 1) {
        const char* argvPath = __system_argv->argv[0];

        if (strncmp(argvPath, "fat:", 4) == 0) {
            // If argv starts by "fat:", check if fat:/ is accessible
            if (access("fat:/", F_OK) == 0) {
                // Extract directory from the full path
                const char* lastSlash = strrchr(argvPath, '/');
                if (lastSlash != NULL) {
                    u32 dirLen = lastSlash - argvPath;
                    if (dirLen < sizeof(romDir)) {
                        strncpy(romDir, argvPath, dirLen);
                        romDir[dirLen] = '\0';
                        return;
                    }
                }
            }
        } else if (strncmp(argvPath, "sd:", 3) == 0) {
            // If argv starts by "sd:", check if sd:/ is accessible
            if (access("sd:/", F_OK) == 0) {
                // Extract directory from the full path
                const char* lastSlash = strrchr(argvPath, '/');
                if (lastSlash != NULL) {
                    u32 dirLen = lastSlash - argvPath;
                    if (dirLen < sizeof(romDir)) {
                        strncpy(romDir, argvPath, dirLen);
                        romDir[dirLen] = '\0';
                        return;
                    }
                }
            }
        }
    }

    // Fallback: try to determine the root directory by checking accessibility
    if (access("sd:/", F_OK) == 0) {
        strncpy(romDir, "sd:", sizeof(romDir) - 1);
        romDir[sizeof(romDir) - 1] = '\0';
        return;
    } else if (access("fat:/", F_OK) == 0) {
        strncpy(romDir, "fat:", sizeof(romDir) - 1);
        romDir[sizeof(romDir) - 1] = '\0';
        return;
    }

    // If all attempts fail, return empty string
    romDir[0] = '\0';
}

bool initFat(void)
{
    if (!fatInitDefault())
        return false;

    loadRomDir();

    // Set CWD to correct ROM root directory
    if (strncmp(romDir, "sd:", 3) == 0)
        chdir("sd:/");
    else if (strncmp(romDir, "fat:", 4) == 0)
        chdir("fat:/");

    return true;
}

char* getRomDir(void)
{
    return romDir;
}

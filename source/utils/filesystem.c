#include "filesystem.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool fileExists(const char* filePath)
{
    struct stat statbuf;
    if (stat(filePath, &statbuf) == 0 && S_ISREG(statbuf.st_mode))
        return true;
    else
        return false;
}

bool dirExists(const char* dirPath)
{
    struct stat statbuf;
    if (stat(dirPath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
        return true;
    else
        return false;
}

bool pathExists(const char* path)
{
    return fileExists(path) || dirExists(path);
}

bool deleteFile(const char* filePath)
{
    if (unlink(filePath) != 0)
        return false;
    return true;
}

bool deleteDir(const char* dirPath)
{
    DIR* dp = opendir(dirPath);
    if (dp == NULL)
        return false;

    struct dirent* entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);

        struct stat statbuf;
        if (stat(fullPath, &statbuf) != 0) {
            closedir(dp);
            return false;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (!deleteDir(fullPath)) {
                closedir(dp);
                return false;
            }
        } else {
            if (!deleteFile(fullPath)) {
                closedir(dp);
                return false;
            }
        }
    }

    closedir(dp);

    if (rmdir(dirPath) != 0)
        return false;

    return true;
}

bool deletePath(const char* path)
{
    return deleteFile(path) || deleteDir(path);
}

bool renamePath(const char* oldPath, const char* newPath)
{
    if (pathExists(newPath)) {
        if (!deletePath(newPath))
            return false;
    }

    if (rename(oldPath, newPath) != 0)
        return false;

    return true;
}

bool createDir(const char* dirPath)
{
    if (mkdir(dirPath, 0755) == 0)
        return true;

    if (errno == EEXIST) {
        if (dirExists(dirPath))
            return true;
    }

    return false;
}

bool createDirR(const char* dirPath)
{
    char tempPath[PATH_MAX];
    char* p = NULL;
    size_t len;

    strncpy(tempPath, dirPath, sizeof(tempPath) - 1);
    len = strlen(tempPath);

    // Remove trailing slash if present
    if (tempPath[len - 1] == '/')
        tempPath[len - 1] = '\0';

    // Skip device name if present (e.g., "fat:")
    p = strchr(tempPath, ':');
    if (p != NULL)
        p++; // Move past the ':'
    else
        p = tempPath;

    // Iterate through the path and create directories as needed
    for (; *p; p++) {
        if (*p != '/')
            continue;

        *p = '\0';

        if (!createDir(tempPath))
            return false;

        *p = '/';
    }

    // Create the final directory
    return createDir(tempPath);
}

static void getPathDir(const char* path, char* dir)
{
    const char* lastSlash = strrchr(path, '/');

    if (!lastSlash) {
        dir[0] = '\0';
        return;
    }

    size_t dirLen = lastSlash - path + 1;
    strncpy(dir, path, dirLen);
    dir[dirLen] = '\0';
}

bool createDirStructure(const char* path)
{
    char dirPath[PATH_MAX];
    getPathDir(path, dirPath);

    return createDirR(dirPath);
}

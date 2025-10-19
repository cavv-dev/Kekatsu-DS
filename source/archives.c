#include "archives.h"

#include "minizip/unzip.h"
#include "utils/filesystem.h"
#include "utils/strings.h"
#include <nds/ndstypes.h>
#include <stdio.h>
#include <string.h>

bool fileIsZip(const char* filePath)
{
    FILE* fp = fopen(filePath, "rb");
    if (!fp)
        return false;

    unsigned char buffer[4];
    size_t bytesRead = fread(buffer, 1, 4, fp);
    fclose(fp);

    return (bytesRead == 4 && buffer[0] == 0x50 && buffer[1] == 0x4b && buffer[2] == 0x03 && buffer[3] == 0x04);
}

static ExtractStatus extractFile(unzFile zipFile, const char* outFilePath)
{
    char zipFilePath[PATH_MAX];
    if (unzGetCurrentFileInfo(zipFile, NULL, zipFilePath, sizeof(zipFilePath), NULL, 0, NULL, 0) != UNZ_OK)
        return EXTRACT_ERR_FILE_READ;

    createDirStructure(outFilePath);

    if (zipFilePath[strlen(zipFilePath) - 1] == '/') {
        if (createDir(outFilePath))
            return EXTRACT_SUCCESS;
        else
            return EXTRACT_ERR_FILE_WRITE;
    }

    FILE* fp = fopen(outFilePath, "wb");
    if (!fp)
        return EXTRACT_ERR_FILE_WRITE;

    char buffer[128];
    int bytesRead;
    while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, bytesRead, 1, fp) != 1) {
            fclose(fp);
            return EXTRACT_ERR_FILE_WRITE;
        }
    }

    fclose(fp);
    return (bytesRead < 0) ? EXTRACT_ERR_FILE_READ : EXTRACT_SUCCESS;
}

ExtractStatus extractZip(const char* filePath, const char* inPath, const char* outPath)
{
    unzFile zipFile = unzOpen(filePath);
    if (!zipFile)
        return EXTRACT_ERR_FILE_OPEN;

    if (inPath[strlen(inPath) - 1] == '/') {
        if (unzGoToFirstFile(zipFile) != UNZ_OK) {
            unzClose(zipFile);
            return EXTRACT_ERR_FILE_READ;
        }

        do {
            char zipFilePath[PATH_MAX];
            if (unzGetCurrentFileInfo(zipFile, NULL, zipFilePath, sizeof(zipFilePath), NULL, 0, NULL, 0) != UNZ_OK) {
                unzClose(zipFile);
                return EXTRACT_ERR_FILE_READ;
            }

            if (strncmp(zipFilePath, inPath, strlen(inPath)) != 0)
                continue;

            char* outDirName = zipFilePath + strlen(inPath);

            if (strlen(outPath) + strlen(outDirName) > PATH_MAX - 1)
                return EXTRACT_ERR_FILE_READ;

            char newOutPath[PATH_MAX];
            joinPath(newOutPath, outPath, outDirName);
            if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
                unzClose(zipFile);
                return EXTRACT_ERR_FILE_READ;
            }

            ExtractStatus result = extractFile(zipFile, newOutPath);
            unzCloseCurrentFile(zipFile);
            if (result != EXTRACT_SUCCESS) {
                unzClose(zipFile);
                return result;
            }
        } while (unzGoToNextFile(zipFile) == UNZ_OK);
    } else {
        if (unzLocateFile(zipFile, inPath, 0) != UNZ_OK) {
            unzClose(zipFile);
            return EXTRACT_ERR_FILE_NOT_FOUND;
        }

        if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
            unzClose(zipFile);
            return EXTRACT_ERR_FILE_READ;
        }

        ExtractStatus result = extractFile(zipFile, outPath);
        unzCloseCurrentFile(zipFile);
        if (result != EXTRACT_SUCCESS) {
            unzClose(zipFile);
            return result;
        }
    }

    unzClose(zipFile);
    return EXTRACT_SUCCESS;
}

ExtractStatus extractAllZip(const char* filePath, const char* outDir)
{
    unzFile zipFile = unzOpen(filePath);
    if (!zipFile)
        return EXTRACT_ERR_FILE_OPEN;

    if (unzGoToFirstFile(zipFile) != UNZ_OK) {
        unzClose(zipFile);
        return EXTRACT_ERR_FILE_READ;
    }

    do {
        if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
            unzClose(zipFile);
            return EXTRACT_ERR_FILE_READ;
        }

        char zipFilePath[PATH_MAX];
        if (unzGetCurrentFileInfo(zipFile, NULL, zipFilePath, sizeof(zipFilePath), NULL, 0, NULL, 0) != UNZ_OK) {
            unzCloseCurrentFile(zipFile);
            unzClose(zipFile);
            return EXTRACT_ERR_FILE_READ;
        }

        if (strlen(outDir) + strlen(zipFilePath) > PATH_MAX - 1)
            return EXTRACT_ERR_FILE_READ;

        char outFilePath[PATH_MAX];
        joinPath(outFilePath, outDir, zipFilePath);

        ExtractStatus result = extractFile(zipFile, outFilePath);
        unzCloseCurrentFile(zipFile);
        if (result != EXTRACT_SUCCESS) {
            unzClose(zipFile);
            return result;
        }
    } while (unzGoToNextFile(zipFile) == UNZ_OK);

    unzClose(zipFile);
    return EXTRACT_SUCCESS;
}

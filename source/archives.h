#pragma once
#include <stdbool.h>

typedef enum {
    EXTRACT_SUCCESS,
    EXTRACT_ERR_FILE_OPEN,
    EXTRACT_ERR_FILE_NOT_FOUND,
    EXTRACT_ERR_FILE_READ,
    EXTRACT_ERR_FILE_WRITE
} ExtractStatus;

bool fileIsZip(const char* filePath);
ExtractStatus extractZip(const char* filePath, const char* inFilePath, const char* outFilePath);
ExtractStatus extractAllZip(const char* filePath, const char* outDir);

#pragma once
#include <stdbool.h>

bool fileExists(const char* filePath);
bool dirExists(const char* dirPath);
bool pathExists(const char* path);
bool deleteFile(const char* filePath);
bool deleteDir(const char* dirPath);
bool deletePath(const char* path);
bool renamePath(const char* oldPath, const char* newPath);
bool createDir(const char* dirPath);
bool createDirR(const char* dirPath);
bool createDirStructure(const char* path);

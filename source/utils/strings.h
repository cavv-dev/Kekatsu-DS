#pragma once
#include <calico/types.h>

void humanizeSize(char* sizeStr, size_t bufferSize, u64 sizeInBytes);
void lowerStr(char*);
void removeAccentsStr(char*);
void safeStr(char*);
void joinPath(char* joinedPath, const char* path1, const char* path2);

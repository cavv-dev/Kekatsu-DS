#pragma once
#include <calico/types.h>

typedef struct Entry* Entry;

struct EntryExtractItem {
    char* outPath;
    char* inPath;
};

Entry newEntry(const char* title, const char* platform, char* region, const char* version, const char* author, const char* url, const char* fileName, u64 size, const char* boxartUrl, struct EntryExtractItem* extractItems, size_t extractItemsCount);
void freeEntry(Entry);
char* getEntryTitle(Entry);
char* getEntryPlatform(Entry);
char* getEntryRegion(Entry);
char* getEntryVersion(Entry);
char* getEntryAuthor(Entry);
char* getEntryUrl(Entry);
char* getEntryFileName(Entry);
u64 getEntrySize(Entry);
char* getEntryBoxartUrl(Entry);
Entry cloneEntry(Entry);
struct EntryExtractItem* getEntryExtractItems(Entry);
size_t getEntryExtractItemsCount(Entry);

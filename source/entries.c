#include "entries.h"

#include <stdlib.h>
#include <string.h>

struct Entry {
    char* title;
    char* platform;
    char* region;
    char* version;
    char* author;
    char* url;
    char* fileName;
    u64 size;
    char* boxartUrl;
    struct EntryExtractItem* extractItems;
    size_t extractItemsCount;
};

Entry newEntry(const char* title, const char* platform, char* region, const char* version, const char* author, const char* url, const char* fileName, u64 size, const char* boxartUrl, struct EntryExtractItem* extractItems, size_t extractItemsCount)
{
    Entry e = malloc(sizeof(struct Entry));
    e->title = strdup(title);
    e->platform = strdup(platform);
    e->region = strdup(region);
    e->version = strdup(version);
    e->author = strdup(author);
    e->url = strdup(url);
    e->fileName = strdup(fileName);
    e->size = size;
    e->boxartUrl = strdup(boxartUrl);

    e->extractItems = malloc(sizeof(struct EntryExtractItem) * extractItemsCount);
    for (int i = 0; i < extractItemsCount; i++) {
        e->extractItems[i].outPath = strdup(extractItems[i].outPath);
        e->extractItems[i].inPath = strdup(extractItems[i].outPath);
    }

    e->extractItemsCount = extractItemsCount;

    return e;
}

void freeEntry(Entry e)
{
    free(e->title);
    free(e->platform);
    free(e->version);
    free(e->author);
    free(e->url);
    free(e->boxartUrl);

    for (int i = 0; i < e->extractItemsCount; i++) {
        free(e->extractItems[i].outPath);
        free(e->extractItems[i].inPath);
    }

    free(e->extractItems);
    free(e);
}

char* getEntryTitle(Entry e)
{
    return e->title;
}

char* getEntryPlatform(Entry e)
{
    return e->platform;
}

char* getEntryRegion(Entry e)
{
    return e->region;
}

char* getEntryVersion(Entry e)
{
    return e->version;
}

char* getEntryAuthor(Entry e)
{
    return e->author;
}

char* getEntryUrl(Entry e)
{
    return e->url;
}

char* getEntryFileName(Entry e)
{
    return e->fileName;
}

u64 getEntrySize(Entry e)
{
    return e->size;
}

char* getEntryBoxartUrl(Entry e)
{
    return e->boxartUrl;
}

Entry cloneEntry(Entry e)
{
    return newEntry(e->title, e->platform, e->region, e->version, e->author, e->url, e->fileName, e->size, e->boxartUrl, e->extractItems, e->extractItemsCount);
}

struct EntryExtractItem* getEntryExtractItems(Entry e)
{
    return e->extractItems;
}

size_t getEntryExtractItemsCount(Entry e)
{
    return e->extractItemsCount;
}

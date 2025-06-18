#include "database.h"

#include "config.h"
#include "networking.h"
#include "utils/filesystem.h"
#include "utils/strings.h"
#include <stdlib.h>
#include <string.h>

#define TEMP_DB_FILENAME "db.txt"
#define TEMP_DB_PATH CACHE_DIR "/" TEMP_DB_FILENAME

#define LAST_OPENED_DB_FILENAME "lastOpenedDb.txt"
#define LAST_OPENED_DB_PATH APPDATA_DIR "/" LAST_OPENED_DB_FILENAME

#define DATABASE_LIST_FILENAME "databases.txt"
#define DATABASE_LIST_PATH APPDATA_DIR "/" DATABASE_LIST_FILENAME

struct Database {
    char* name;
    char* value;
    DatabaseType type;
    char* path;
    size_t size;
    bool isInited;
    FILE* fp;
    char delimiter;
};

Database newDatabase(const char* name, const char* value)
{
    Database d = malloc(sizeof(struct Database));
    d->name = strdup(name);
    d->value = strdup(value);
    d->path = NULL;
    d->size = 0;
    d->isInited = false;
    d->fp = NULL;
    d->delimiter = '\0';

    if ((strncmp(value, "http://", 7) == 0) || (strncmp(value, "https://", 8) == 0))
        d->type = DATABASE_TYPE_HTTP;
    else
        d->type = DATABASE_TYPE_LOCAL;

    return d;
}

void freeDatabase(Database d)
{
    free(d->value);
    free(d->path);

    if (d->fp)
        closeDatabase(d);

    free(d);
}

char* getDatabaseName(Database d)
{
    return d->name;
}

char* getDatabaseValue(Database d)
{
    return d->value;
}

char* getDatabasePath(Database d)
{
    return d->path;
}

size_t getDatabaseSize(Database d)
{
    return d->size;
}

bool getDatabaseIsInited(Database d)
{
    return d->isInited;
}

char** parseLine(char* line, char delimiter, size_t* fieldsCount)
{
    if (line[0] == '\0') {
        *fieldsCount = 0;
        return NULL;
    }

    // Count fields in line
    *fieldsCount = 1;
    for (char* ptr = line; *ptr != '\0'; ++ptr) {
        if (*ptr == delimiter)
            (*fieldsCount)++;
    }

    // Allocate space for fields array
    char** fields = (char**)malloc(*fieldsCount * sizeof(char*));
    if (fields == NULL)
        return NULL;

    // Populate fields array with pointers to strings in line
    size_t fieldIndex = 0;
    char* start = line;
    for (char* ptr = line;; ++ptr) {
        if (*ptr == delimiter || *ptr == '\0') {
            fields[fieldIndex] = start;
            fieldIndex++;
            if (*ptr == '\0') {
                break;
            }
            *ptr = '\0'; // Replace delimiter with null terminator
            start = ptr + 1;
        }
    }

    return fields;
}

Database getLastOpenedDatabase(void)
{
    FILE* fp = fopen(LAST_OPENED_DB_PATH, "r");
    if (!fp)
        return NULL;

    char line[2048];
    char name[1024];
    char value[1024];

    if (!fgets(line, sizeof(line), fp)
        || (sscanf(line, "%[^\t]\t%s", name, value) != 2
            && sscanf(line, "%[^=]=%s", name, value) != 2)) {
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    return newDatabase(name, value);
}

void saveLastOpenedDatabase(Database d)
{
    FILE* fp = fopen(LAST_OPENED_DB_PATH, "w");
    if (!fp)
        return;

    fprintf(fp, "%s\t%s\n", d->name, d->value);
    fclose(fp);
}

DatabaseInitStatus openDatabase(Database d)
{
    FILE* fp = fopen(d->path, "r");
    if (!fp)
        return DATABASE_OPEN_ERR_FILE_OPEN;

    size_t dbSize = 0;

    char line[1024];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return DATABASE_OPEN_ERR_INVALID_FORMAT;
    }

    int dbVersion = atoi(line);
    if (dbVersion != 1) { // 1 is the only supported version for now
        fclose(fp);
        return DATABASE_OPEN_ERR_INVALID_VERSION;
    }

    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return DATABASE_OPEN_ERR_INVALID_FORMAT;
    }

    char delimiter = line[0];

    while (fgets(line, sizeof(line), fp)) {
        size_t fieldsCount;
        char** fields = parseLine(line, delimiter, &fieldsCount);

        if (!fields || fieldsCount < 9) {
            fclose(fp);
            return DATABASE_OPEN_ERR_INVALID_FORMAT;
        }

        // Calculate the fields count of items to extract
        // Items are couple of fields so the count has to be an even number
        size_t extractItemsFieldsCount = fieldsCount - 9;
        if (fieldsCount > 9 && (extractItemsFieldsCount & 1) != 0) {
            free(fields);
            fclose(fp);
            return DATABASE_OPEN_ERR_INVALID_FORMAT;
        }

        dbSize++;
    }

    d->size = dbSize;
    d->fp = fp;
    d->delimiter = delimiter;

    saveLastOpenedDatabase(d);

    return DATABASE_OPEN_SUCCESS;
}

bool closeDatabase(Database d)
{
    if (fclose(d->fp) != 0)
        return false;

    return true;
}

DatabaseInitStatus initDatabase(Database d)
{
    bool usingCachedDb = false;

    switch (d->type) {
    case DATABASE_TYPE_HTTP:
        char* dbFileName = strdup(d->value);
        safeStr(dbFileName);

        char dbFilePath[PATH_MAX];
        joinPath(dbFilePath, CACHE_DIR, dbFileName);
        free(dbFileName);

        d->path = strdup(dbFilePath);

        if (downloadFile(TEMP_DB_PATH, d->value, NULL) == DOWNLOAD_SUCCESS) {
            if (!renamePath(TEMP_DB_PATH, dbFilePath))
                return DATABASE_INIT_ERR_DOWNLOAD;
        } else {
            if (fileExists(dbFilePath))
                usingCachedDb = true;
            else
                return DATABASE_INIT_ERR_DOWNLOAD;
        }

        break;
    case DATABASE_TYPE_LOCAL:
        d->path = strdup(d->value);

        if (!pathExists(d->path))
            return DATABASE_INIT_ERR_FILE_NOT_FOUND;

        break;
    }

    d->isInited = true;

    return (usingCachedDb ? DATABASE_INIT_SUCCESS_CACHE : DATABASE_INIT_SUCCESS);
}

void alignDatabase(Database d)
{
    fseek(d->fp, 0, SEEK_SET);

    // Advance 2 lines
    char ch;
    u8 lines = 2;
    while (lines > 0 && (ch = fgetc(d->fp)) != EOF) {
        if (ch == '\n') {
            lines--;
        }
    }
}

Entry* searchDatabase(Database d, const char* searchTitle, size_t pageSize, size_t page, size_t* resultsCount)
{
    *resultsCount = 0;
    size_t count = 0;
    size_t capacity = 10;
    size_t startIndex = (page - 1) * pageSize;
    size_t endIndex = startIndex + pageSize;
    size_t currIndex = 0;
    char line[1024];
    Entry* results = malloc(capacity * sizeof(Entry));

    alignDatabase(d);

    while (fgets(line, sizeof(line), d->fp)) {
        // Trim trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        size_t fieldsCount;
        char** fields = parseLine(line, d->delimiter, &fieldsCount);

        if (searchTitle[0] != '\0') {
            // Format titles in a comparable way
            char tempEntryTitle[128];
            char tempInputTitle[128];

            strncpy(tempEntryTitle, fields[0], sizeof(tempEntryTitle) - 1);
            removeAccentsStr(tempEntryTitle);
            lowerStr(tempEntryTitle);

            strncpy(tempInputTitle, searchTitle, sizeof(tempInputTitle) - 1);
            lowerStr(tempInputTitle);

            if (!strstr(tempEntryTitle, tempInputTitle)) {
                free(fields);
                continue;
            }
        }

        if (currIndex < startIndex || currIndex >= endIndex) {
            currIndex++;
            continue;
        }

        char** extractItemsFields = &fields[9]; // Items to extract start from the ninth field

        size_t extractItemsCount = (fieldsCount - 9) / 2;
        struct EntryExtractItem extractItems[extractItemsCount];

        for (size_t i = 0; i < extractItemsCount; i++) {
            extractItems[i].outPath = extractItemsFields[i * 2];
            extractItems[i].inPath = extractItemsFields[i * 2 + 1];
        }

        Entry e = newEntry(fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], strtoull(fields[7], NULL, 10), fields[8], extractItems, extractItemsCount);

        // Increase the capacity if needed
        if (count >= capacity) {
            capacity *= 2;
            results = realloc(results, capacity * sizeof(Entry));
        }

        results[count] = e;
        count++;

        currIndex++;
        free(fields);

        if (currIndex >= endIndex)
            break;
    }

    *resultsCount = count;
    return results;
}

Database* getDatabaseList(size_t* databasesCount)
{
    *databasesCount = 0;

    FILE* fp = fopen(DATABASE_LIST_PATH, "r");
    if (!fp)
        return NULL;

    size_t count = 0;
    size_t capacity = 8;
    Database* databases = malloc(capacity * sizeof(Database));

    char line[2048];
    char name[1024];
    char value[1024];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%[^\t]\t%s", name, value) != 2
            && sscanf(line, "%[^=]=%s", name, value) != 2)
            continue;

        if (count >= capacity) {
            capacity *= 2;
            databases = realloc(databases, capacity * sizeof(Database));
        }

        databases[count++] = newDatabase(name, value);
    }

    fclose(fp);
    *databasesCount = count;
    return databases;
}

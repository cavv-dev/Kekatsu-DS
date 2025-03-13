#pragma once
#include "entries.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Database* Database;

typedef enum {
    DATABASE_TYPE_HTTP,
    DATABASE_TYPE_LOCAL
} DatabaseType;

typedef enum {
    DATABASE_INIT_SUCCESS,
    DATABASE_INIT_SUCCESS_CACHE,
    DATABASE_INIT_ERR_DOWNLOAD,
    DATABASE_INIT_ERR_FILE_NOT_FOUND
} DatabaseInitStatus;

typedef enum {
    DATABASE_OPEN_SUCCESS,
    DATABASE_OPEN_ERR_FILE_OPEN,
    DATABASE_OPEN_ERR_INVALID_VERSION,
    DATABASE_OPEN_ERR_INVALID_FORMAT
} DatabaseOpenStatus;

Database newDatabase(const char* name, const char* value);
void freeDatabase(Database);
char* getDatabaseName(Database);
char* getDatabaseValue(Database);
char* getDatabasePath(Database);
size_t getDatabaseSize(Database);
bool getDatabaseIsInited(Database);
Database getLastOpenedDatabase(void);
DatabaseInitStatus openDatabase(Database);
bool closeDatabase(Database d);
DatabaseInitStatus initDatabase(Database);
Entry* searchDatabase(Database, const char* searchTitle, size_t pageSize, size_t page, size_t* resultsCount);
Database* getDatabaseList(size_t* databasesCount);

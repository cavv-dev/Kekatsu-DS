#include "networking.h"

#include "config.h"
#include <dswifi9.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024 * 1024 // 1 MiB
#define USER_AGENT APP_NAME "/" APP_VERSION " (DS)"

NetworkingInitStatus initNetworking(void)
{
    if (!Wifi_InitDefault(WFC_CONNECT | WIFI_ATTEMPT_DSI_MODE))
        return NETWORKING_INIT_ERR_WIFI_CONNECT;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    return NETWORKING_INIT_SUCCESS;
}

bool stopDownloadSignal = false;

struct WriteData {
    FILE* fp;
    char* buffer;
    size_t bufferPos;
};

size_t writeDataCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    struct WriteData* writeData = (struct WriteData*)userdata;
    if (stopDownloadSignal)
        return -1;

    size_t total_size = size * nmemb;
    if (writeData->bufferPos + total_size > BUFFER_SIZE) {
        fwrite(writeData->buffer, 1, writeData->bufferPos, writeData->fp);
        writeData->bufferPos = 0;
    }

    if (total_size > BUFFER_SIZE) {
        fwrite(ptr, 1, total_size, writeData->fp);
    } else {
        memcpy(writeData->buffer + writeData->bufferPos, ptr, total_size);
        writeData->bufferPos += total_size;
    }

    return total_size;
}

DownloadStatus downloadFile(const char* path, const char* url, size_t (*downloadProgressCallback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t))
{
    stopDownloadSignal = false;

    CURL* curl = curl_easy_init();
    if (!curl)
        return DOWNLOAD_ERR_INIT_FAILED;

    FILE* fp = fopen(path, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return DOWNLOAD_ERR_FILE_OPEN_FAILED;
    }

    struct WriteData writeData;
    writeData.fp = fp;
    writeData.buffer = (char*)malloc(BUFFER_SIZE);
    writeData.bufferPos = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7L);

    if (downloadProgressCallback) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, downloadProgressCallback);
    }

    CURLcode res = curl_easy_perform(curl);

    if (writeData.bufferPos > 0)
        fwrite(writeData.buffer, 1, writeData.bufferPos, writeData.fp);

    free(writeData.buffer);
    fclose(fp);

    if (stopDownloadSignal) {
        curl_easy_cleanup(curl);
        unlink(path);
        return DOWNLOAD_STOPPED;
    }

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        unlink(path);
        return DOWNLOAD_ERR_PERFORM_FAILED;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode != 200) {
        unlink(path);
        return DOWNLOAD_ERR_NOT_OK;
    }

    return DOWNLOAD_SUCCESS;
}

void stopDownload(void)
{
    stopDownloadSignal = true;
}

struct MemoryWriteData {
    char* result;
    size_t bufferSize;
    size_t currentPos;
};

size_t memoryWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    struct MemoryWriteData* writeData = (struct MemoryWriteData*)userdata;
    size_t total_size = size * nmemb;

    if (writeData->currentPos + total_size >= writeData->bufferSize) {
        total_size = writeData->bufferSize - writeData->currentPos - 1;
    }

    memcpy(writeData->result + writeData->currentPos, ptr, total_size);
    writeData->currentPos += total_size;
    writeData->result[writeData->currentPos] = '\0';

    return total_size;
}

DownloadStatus downloadToString(char* result, size_t bufferSize, const char* url)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return DOWNLOAD_ERR_INIT_FAILED;

    struct MemoryWriteData writeData;
    writeData.result = result;
    writeData.bufferSize = bufferSize;
    writeData.currentPos = 0;
    result[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memoryWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return DOWNLOAD_ERR_PERFORM_FAILED;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode != 200)
        return DOWNLOAD_ERR_NOT_OK;

    return DOWNLOAD_SUCCESS;
}

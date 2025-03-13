#pragma once
#include <curl/curl.h>

typedef enum {
    NETWORKING_INIT_SUCCESS,
    NETWORKING_INIT_ERR_WIFI_CONNECT
} NetworkingInitStatus;

typedef enum {
    DOWNLOAD_SUCCESS,
    DOWNLOAD_STOPPED,
    DOWNLOAD_ERR_NOT_OK,
    DOWNLOAD_ERR_INIT_FAILED,
    DOWNLOAD_ERR_FILE_OPEN_FAILED,
    DOWNLOAD_ERR_PERFORM_FAILED
} DownloadStatus;

NetworkingInitStatus initNetworking(void);
DownloadStatus downloadFile(const char* path, const char* url, size_t (*downloadProgressCallback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t));
void stopDownload(void);
DownloadStatus downloadToString(char* result, size_t bufferSize, const char* url);

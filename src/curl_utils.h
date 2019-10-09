#ifndef CURL_UTILS_H_INCLUDED
#define CURL_UTILS_H_INCLUDED

#define MAX_LINE_LENGTH 8192
#define USERAGENT "curl"
#define CREDENTIALS_FILE "credentials.txt"
#define SAVEFILES "savefiles.txt"

typedef struct {
    char *data;
    size_t size;
} CurlReturn;

typedef struct FileList {
    char *path;
    char *name;
    struct FileList *next;
} FileList;

void curl_init();
void curl_cleanup();

#endif

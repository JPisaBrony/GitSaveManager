#ifndef CURL_UTILS_H_INCLUDED
#define CURL_UTILS_H_INCLUDED

#define USERAGENT "curl"

typedef struct {
    char *data;
    size_t size;
} CurlReturn;

void createGist(char* path, char* filename);
void getAndSaveGist(char* filename, char* url);
char* getUrlFromGistByFilename(char *filename);
void updateGist(char* filename, char* path, char* url);
void curl_cleanup();
void curl_init();

#endif

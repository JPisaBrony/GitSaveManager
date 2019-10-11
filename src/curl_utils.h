#ifndef CURL_UTILS_H_INCLUDED
#define CURL_UTILS_H_INCLUDED

#define USERAGENT "curl"

typedef struct {
    char *data;
    size_t size;
} CurlReturn;

void curl_init();
void curl_cleanup();

#endif

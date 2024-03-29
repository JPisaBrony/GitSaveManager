#include "global.h"

int check_username_and_password() {
    if(username == NULL || username[0] == '\0')
        return -1;
    if(password == NULL || password[0] == '\0')
        return -1;
    return 0;
}

static size_t CurlReturnCallback(void *data, size_t size, size_t nmemb, void *curl_ret) {
    size_t realsize = size * nmemb;
    CurlReturn *ret = (CurlReturn *) curl_ret;

    char *ptr = realloc(ret->data, ret->size + realsize + 1);

    if(ptr == NULL) {
        printf("not enough memory\n");
    } else {
        ret->data = ptr;
        memcpy(&(ret->data[ret->size]), data, realsize);
        ret->size += realsize;
        ret->data[ret->size] = 0;

        return realsize;
    }

    return 0;
}

int createGist(char* path, char* filename) {
    if(check_username_and_password() == -1)
        return -1;

    CurlReturn curl_ret;
    curl_ret.data = malloc(1);
    curl_ret.size = 0;

    char* buffer = open_file_and_get_hex_string(path);
    if(buffer == NULL)
        return -1;

    // construct json object for PATCH request
    json_object *json = json_object_new_object();
    json_object *json_file = json_object_new_object();
    json_object *json_filename = json_object_new_object();
    json_object *content_val = json_object_new_string(buffer);
    json_object_object_add(json_file, "content", content_val);
    json_object_object_add(json_filename, filename, json_file);
    json_object_object_add(json, "files", json_filename);

    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/gists");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlReturnCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &curl_ret);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }

    // cleanup
    free(json_file);
    free(json_filename);
    free(content_val);
    free(json);
    free(buffer);
    free(curl_ret.data);
    return 0;
}

int getAndSaveGist(char* filename, char* url) {
    if(check_username_and_password() == -1)
        return -1;

    CurlReturn curl_ret;
    curl_ret.data = malloc(1);
    curl_ret.size = 0;

    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlReturnCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &curl_ret);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        json_object *json_obj = json_tokener_parse(curl_ret.data);
        json_obj = json_object_object_get(json_obj, "files");
        lh_table *files_table = json_object_get_object(json_obj);
        const char *file_name = files_table->head->k;
        json_obj = json_object_object_get(json_obj, file_name);
        json_obj = json_object_object_get(json_obj, "content");
        char* content = (char*) json_object_to_json_string(json_obj);
        // remove first char
        content++;
        // remove last char
        int content_len = strlen(content);
        content[content_len-1] = 0;

        hex_to_binary(content, content_len / 2);

        FILE *file = NULL;
        file = fopen(filename, "wb");
        fwrite(content, content_len / 2, 1, file);
        fclose(file);

        // cleanup
        free(json_obj);
        free(files_table);
        curl_easy_cleanup(curl);
    }

    free(curl_ret.data);
    return 0;
}

char* getUrlFromGistByFilename(char *filename) {
    CurlReturn curl_ret;
    curl_ret.data = malloc(1);
    curl_ret.size = 0;

    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/gists");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlReturnCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &curl_ret);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        json_object *json_obj = json_tokener_parse(curl_ret.data);

        if(json_object_is_type(json_obj, json_type_array)) {
            int i;
            json_object *json_iter = NULL;
            size_t json_obj_len = json_object_array_length(json_obj);
            for(i = 0; i < json_obj_len; i++) {
                json_iter = json_object_array_get_idx(json_obj, i);
                json_iter = json_object_object_get(json_iter, "files");
                lh_table *files_table = json_object_get_object(json_iter);
                const char *filename_json = files_table->head->k;
                if(strcmp(filename, filename_json) == 0) {
                    json_iter = json_object_array_get_idx(json_obj, i);
                    json_iter = json_object_object_get(json_iter, "url");
                    free(curl_ret.data);
                    curl_easy_cleanup(curl);
                    return (char *) json_object_get_string(json_iter);
                }
            }
            free(json_iter);
        } else {
            return "e";
        }

        curl_easy_cleanup(curl);
    }

    free(curl_ret.data);
    return NULL;
}

int updateGist(char* filename, char* path, char* url) {
    if(check_username_and_password() == -1)
        return -1;

    CurlReturn curl_ret;
    curl_ret.data = malloc(1);
    curl_ret.size = 0;

    char* buffer = open_file_and_get_hex_string(path);
    if(buffer == NULL)
        return -1;

    // construct json object for PATCH request
    json_object *json = json_object_new_object();
    json_object *json_file = json_object_new_object();
    json_object *json_filename = json_object_new_object();
    json_object *content_val = json_object_new_string(buffer);
    json_object_object_add(json_file, "content", content_val);
    json_object_object_add(json_filename, filename, json_file);
    json_object_object_add(json, "files", json_filename);

    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlReturnCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &curl_ret);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }

    // cleanup
    free(json_file);
    free(json_filename);
    free(content_val);
    free(json);
    free(buffer);
    free(curl_ret.data);
    return 0;
}

void curl_cleanup() {
    curl_global_cleanup();
}

void curl_init() {
    curl_global_init(CURL_GLOBAL_ALL);
}

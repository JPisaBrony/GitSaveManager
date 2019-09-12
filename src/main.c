#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#ifdef _3DS
#include <3ds.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

#define SIZE 256
int getline(char **lineptr, size_t *n, FILE *stream) {
    static char line[SIZE];
    char *ptr;
    unsigned int len;

    if (lineptr == NULL || n == NULL)
        return -1;

    if (ferror (stream))
        return -1;

    if (feof(stream))
        return -1;

    fgets(line, SIZE, stream);

    ptr = strchr(line,'\n');
    if (ptr)
      *ptr = '\0';

    len = strlen(line);

    if ((len + 1) < SIZE) {
      ptr = realloc(*lineptr, SIZE);
      if (ptr == NULL)
         return(-1);
      *lineptr = ptr;
      *n = SIZE;
    }

    strcpy(*lineptr,line);
    return(len);
}

#endif

#define USERAGENT "curl"
#define CREDENTIALS_FILE "credentials.txt"
#define SAVEFILES "savefiles.txt"

char* username = NULL;
char* password = NULL;

typedef struct {
    char *data;
    size_t size;
} CurlReturn;

typedef struct FileList {
    char *path;
    char *name;
    struct FileList *next;
} FileList;

char* STR_concat(char* str0, char* str1) {
    int len0 = strlen(str0);
    int len1 = strlen(str1);
    int len = len0 + len1;
    if(len > 999999999) {
        return NULL;
    }
    len+=1;
    char* buffer = malloc(len);

    strcpy(buffer, str0);
    strcat(buffer, str1);
    buffer[len-1]='\0';

    return buffer;
}

void getLocalCreds() {
    int i;
    FILE *file = NULL;
    char *line = NULL;
    size_t len = 0;

    file = fopen(CREDENTIALS_FILE, "r");

    if(file == NULL) {
        printf("failed to open file %s\n", CREDENTIALS_FILE);
        return;
    }

    i = 0;
    while(getline(&line, &len, file) != -1) {
        if(i == 0) {
            username = malloc(len - 1);
            strcpy(username, strtok(line, "\n"));
            i++;
        } else if(i == 1) {
            password = malloc(len - 1);
            strcpy(password, strtok(line, "\n"));
            i++;
        }
    }

    fclose(file);
    free(line);
}

void freeCreds() {
    free(username);
    free(password);
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

void hexToBinary(char *buffer, int binarySize) {
    int i;
    char raw;
    char vals[256];

    for(i = 0; i < 10; i++){
        vals['0'+i] = i;
    }
    for(i = 0; i < 6; i++){
        vals['A'+i] = 10+i;
        vals['a'+i] = 10+i;
    }

    for(i = 0; i < binarySize - 1; i++) {
        raw = vals[buffer[i<<1]]<<4;
        raw |= vals[buffer[(i<<1)+1]];
        buffer[i] = raw;
    }
}

void binaryToHex(char *doubleSizedCharBuffer, int binarySize) {
	const char hex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	int i;
	char buff;

	for(i=binarySize-1; i > -1; i--){
		buff = doubleSizedCharBuffer[i];
		doubleSizedCharBuffer[i<<1] = hex[(buff>>4) & 0xF];
		doubleSizedCharBuffer[(i<<1)+1] = hex[buff & 0xF];
    }
}

char* openFileAndGetHexString(char* filename) {
    int i, c;
    FILE *file = NULL;
    file = fopen(filename, "rb");

    if(file == NULL) {
        printf("failed to open file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int binarySize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(binarySize << 2);

    i = 0;
    while((c = fgetc(file)) != EOF) {
        buffer[i] = c;
        i++;
    }

    fclose(file);

    binaryToHex(buffer, binarySize);

    return buffer;
}

void createGist(char* path, char* filename) {
    char* buffer = openFileAndGetHexString(path);
    if(buffer == NULL)
        return;

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
}

void getAndSaveGist(char* filename, char* url) {
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

        hexToBinary(content, content_len);

        FILE *file = NULL;
        file = fopen(filename, "wb");
        fwrite(content, content_len / 2, 1, file);
        fclose(file);

        // cleanup
        free(json_obj);
        free(files_table);
        free(curl_ret.data);
        curl_easy_cleanup(curl);
    }
}

/*
unused for now
void getStarredGist() {
    CurlReturn curl_ret;
    curl_ret.data = malloc(1);
    curl_ret.size = 0;

    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/gists/starred");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlReturnCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &curl_ret);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        json_object *json_obj = json_tokener_parse(curl_ret.data);

        int i;
        json_object *json_iter;
        size_t json_obj_len = json_object_array_length(json_obj);

        for(i = 0; i < json_obj_len; i++) {
            json_iter = json_object_array_get_idx(json_obj, i);
            json_iter = json_object_object_get(json_iter, "url");
            char* url = (char *) json_object_get_string(json_iter);
            getAndSaveGist(url);
        }

        free(json_iter);
        curl_easy_cleanup(curl);
    }
}
*/

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

        int i;
        json_object *json_iter;
        size_t json_obj_len = json_object_array_length(json_obj);

        for(i = 0; i < json_obj_len; i++) {
            json_iter = json_object_array_get_idx(json_obj, i);
            json_iter = json_object_object_get(json_iter, "files");
            lh_table *files_table = json_object_get_object(json_iter);
            const char *filename_json = files_table->head->k;
            if(strcmp(filename, filename_json) == 0) {
                json_iter = json_object_array_get_idx(json_obj, i);
                json_iter = json_object_object_get(json_iter, "url");
                return (char *) json_object_get_string(json_iter);
            }
        }

        free(json_iter);
        curl_easy_cleanup(curl);
    }

    return NULL;
}

void updateGist(char* filename, char* url) {
    char* buffer = openFileAndGetHexString(filename);
    if(buffer == NULL)
        return;

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
}

void exit_msg(char* msg) {
    printf("%s\n", msg);
    exit(-1);
}

void exit_msg_cmd() {
    printf("Add either g for get or p push to the command line\n");
    exit(-1);
}

FileList* getSavefileList() {
    FILE *file = NULL;
    char *line = NULL;
    size_t len = 0;

    file = fopen(SAVEFILES, "r");

    if(file == NULL) {
        printf("failed to open file %s\n", SAVEFILES);
        return;
    }

    FileList *file_list = malloc(sizeof(FileList));
    FileList *iter = file_list;

    while(getline(&line, &len, file) != -1) {
        FileList *new_node = malloc(sizeof(FileList));
        char* split = strtok(line, " ");
        new_node->path = malloc(strlen(split));
        strcpy(new_node->path, split);
        split = strtok(NULL, " ");
        new_node->name = malloc(strlen(split) - 1);
        strcpy(new_node->name, strtok(split, "\n"));
        new_node->next = NULL;
        iter->next = new_node;
        iter = new_node;
    }

    // save first node
    iter = file_list;
    // fix first node to point to correct one
    file_list = file_list->next;
    // free unlinked node
    free(iter);

    printf("currently tracked files\n");
    FileList *cur = file_list;
    while(cur != NULL) {
        printf("%s\n", cur->path);
        cur = cur->next;
    }
    printf("\n");

    fclose(file);
    free(line);

    return file_list;
}

void freeSavefileList(FileList *file_list) {
    FileList *cur = file_list;
    FileList *temp;
    while(cur != NULL) {
        temp = cur;
        cur = cur->next;
        free(temp);
    }
}

int main(int argc, char *argv[]) {
    #ifdef _3DS
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    // allocate buffer for SOC service
    u32 *SOC_buffer = (u32*) memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    int ret;

    if(SOC_buffer == NULL) {
        printf("memalign: failed to allocate\n");
        return -1;
    }

    // Now intialise soc:u service
    if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
        printf("socInit: 0x%08X\n", (unsigned int) ret);
        return -1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    getLocalCreds();
    FileList *file_list = getSavefileList();

    printf("Press A to upload files\n");
    printf("Press B to download files\n");

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        if(kDown & KEY_A) {
            FileList *cur = file_list;
            while(cur != NULL) {
                char* url = getUrlFromGistByFilename(cur->name);
                if(url == NULL)
                    createGist(cur->path, cur->name);
                else
                    updateGist(cur->path, url);

                cur = cur->next;
            }
            printf("upload successful\n");
        } else if(kDown & KEY_B) {
            FileList *cur = file_list;
            while(cur != NULL) {
                char* url = getUrlFromGistByFilename(cur->name);
                if(url != NULL)
                    getAndSaveGist(cur->path, url);

                cur = cur->next;
            }

            printf("download successful\n");
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    freeSavefileList(file_list);
    curl_global_cleanup();
    freeCreds();
    socExit();
    gfxExit();

    #else

    if(argc == 1) {
        exit_msg_cmd();
    } else if(argv[1][0] == 'p') {
        curl_global_init(CURL_GLOBAL_ALL);
        getLocalCreds();
        FileList *file_list = getSavefileList();

        FileList *cur = file_list;
        while(cur != NULL) {
            char* url = getUrlFromGistByFilename(cur->name);
            if(url == NULL)
                createGist(cur->path, cur->name);
            else
                updateGist(cur->path, url);

            cur = cur->next;
        }

        freeSavefileList(file_list);
        curl_global_cleanup();
        freeCreds();
    } else if(argv[1][0] == 'g') {
        curl_global_init(CURL_GLOBAL_ALL);
        getLocalCreds();
        FileList *file_list = getSavefileList();

        FileList *cur = file_list;
        while(cur != NULL) {
            char* url = getUrlFromGistByFilename(cur->name);
            if(url != NULL)
                getAndSaveGist(cur->path, url);

            cur = cur->next;
        }

        freeSavefileList(file_list);
        curl_global_cleanup();
        freeCreds();
    } else {
        exit_msg_cmd();
    }

    #endif

    return 0;
}

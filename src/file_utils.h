#ifndef FILE_UTILS_C_INCLUDED
#define FILE_UTILS_C_INCLUDED

#define MAX_LINE_LENGTH 8192
#define CREDENTIALS_FILE "credentials.txt"
#define SAVEFILES "savefiles.txt"

typedef struct FileList {
    char *path;
    char *name;
    struct FileList *next;
} FileList;

void file_init();
void file_cleanup();
char* get_username();
char* get_password();
FileList* get_filelist();
char* open_file_and_get_hex_string(char* filename);
void append_data_to_save_file(char* data);
void hex_to_binary(char *doubleSizedCharBuffer, int binarySize);
void hex_to_binary(char *buffer, int binarySize);

#endif

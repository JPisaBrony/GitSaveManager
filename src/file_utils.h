#ifndef FILE_UTILS_C_INCLUDED
#define FILE_UTILS_C_INCLUDED

#define MAX_LINE_LENGTH 8192
#define CREDENTIALS_FILE "credentials.txt"
#define SAVEFILES "savefiles.txt"
#define LOCAL_CREDS_STATUS_OK 0
#define LOCAL_CREDS_STATUS_FAILURE -1

typedef struct FileList {
    char *path;
    char *name;
    struct FileList *next;
} FileList;

void file_init();
void file_cleanup();
void write_local_creds();
void delete_local_creds();
void delete_local_save_file();
void get_filelist();
FileList* create_filelist_node(char* path, char* name);
void free_filelist_node(FileList *node);
char* open_file_and_get_hex_string(char* filename);
void append_node_to_save_file(FileList *node);
void write_save_file_from_filelist(FileList *list);
void delete_node_from_filelist(FileList **list, int node_number);
void hex_to_binary(char *doubleSizedCharBuffer, int binarySize);
void hex_to_binary(char *buffer, int binarySize);

#endif

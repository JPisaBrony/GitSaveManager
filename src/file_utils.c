#include "global.h"

char* username = NULL;
char* password = NULL;
FileList *filelist = NULL;

char* get_username() {
    return username;
}

char* get_password() {
    return password;
}

void hex_to_binary(char *buffer, int binarySize) {
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

void binary_to_hex(char *doubleSizedCharBuffer, int binarySize) {
	const char hex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	int i;
	char buff;

	for(i=binarySize-1; i > -1; i--){
		buff = doubleSizedCharBuffer[i];
		doubleSizedCharBuffer[i<<1] = hex[(buff>>4) & 0xF];
		doubleSizedCharBuffer[(i<<1)+1] = hex[buff & 0xF];
    }
}

char* open_file_and_get_hex_string(char* filename) {
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

    binary_to_hex(buffer, binarySize);

    return buffer;
}

FileList* create_filelist_node(char* path, char* name) {
    FileList *node = malloc(sizeof(FileList));
    node->path = malloc(strlen(path) + 1);
    strcpy(node->path, path);
    node->name = malloc(strlen(name) + 1);
    strcpy(node->name, name);
    node->next = NULL;
    return node;
}

FileList* get_filelist_from_save_file() {
    FILE *file = NULL;
    char *line = malloc(MAX_LINE_LENGTH);
    char* name;
    char* path;

    file = fopen(SAVEFILES, "r");

    if(file == NULL)
        return NULL;

    FileList *list = malloc(sizeof(FileList));
    FileList *iter = list;

    while(fgets(line, MAX_LINE_LENGTH, file)) {
        name = strtok(line, " ");
        path = strtok(NULL, " ");
        path = strtok(path, "\n");
        FileList *new_node = create_filelist_node(path, name);
        iter->next = new_node;
        iter = new_node;
    }

    // save first node
    iter = list;
    // fix first node to point to correct one
    list = list->next;
    // free unlinked node
    free(iter);

    // cleanup
    fclose(file);
    free(line);

    return list;
}

void write_save_file_from_filelist(FileList *list) {
    FILE *file = NULL;
    char *line;
    FileList *cur = list;

    file = fopen(SAVEFILES, "w");

    if(file == NULL)
        exit_msg("failed to open file " SAVEFILES);

    while(cur != NULL) {
        line = malloc(MAX_LINE_LENGTH);
        strcpy(line, cur->name);
        strcat(line, " ");
        strcat(line, cur->path);
        fputs(line, file);
        free(line);
        cur = cur->next;
    }

    // cleanup
    fclose(file);
}

void append_node_to_save_file(FileList *node) {
    if(node == NULL)
        return;

    FILE *file = NULL;

    file = fopen(SAVEFILES, "a");

    if(file == NULL)
        exit_msg("failed to open file " SAVEFILES);

    char *line = malloc(MAX_LINE_LENGTH);
    strcpy(line, node->name);
    strcat(line, " ");
    strcat(line, node->path);
    strcat(line, node->name);
    strcat(line, "\n");
    fputs(line, file);
    free(line);

    fclose(file);
}

void delete_node_from_filelist(FileList **list, int node_number) {
    int i = 0;
    FileList *cur = *list;
    FileList *temp = NULL;

    if(*list == NULL)
        return;

    if(node_number == 0) {
        temp = (*list)->next;
        free(*list);
        *list = temp;
        return;
    }

    while(cur != NULL) {
        if(i == node_number) {
            temp->next = cur->next;
            free(cur);
            break;
        }
        temp = cur;
        cur = cur->next;
        i++;
    }
}

void free_filelist_node(FileList *node) {
    free(node->path);
    free(node->name);
    free(node);
}

void free_filelist() {
    FileList *cur = filelist;
    FileList *temp;
    while(cur != NULL) {
        temp = cur;
        cur = cur->next;
        free_filelist_node(temp);
    }
}

void get_local_creds() {
    int i = 0;
    FILE *file = NULL;
    char *line = malloc(MAX_LINE_LENGTH);
    size_t len = 0;

    file = fopen(CREDENTIALS_FILE, "r");

    if(file == NULL) {
        free(line);
        printf("failed to open file %s\n", CREDENTIALS_FILE);
        return;
    }

    while(fgets(line, MAX_LINE_LENGTH, file)) {
        len = strlen(line);
        if(i == 0) {
            username = malloc(len - 1);
            strcpy(username, strtok(line, "\r\n"));
            i++;
        } else if(i == 1) {
            password = malloc(len - 1);
            strcpy(password, strtok(line, "\r\n"));
            i++;
        }
    }

    fclose(file);
    free(line);
}

FileList* get_filelist() {
    // reset the filelist incase it changed
    free_filelist();
    // get the filelist
    filelist = get_filelist_from_save_file();
    // return the filelist if it exists
    // otherwise the filelist will be NULL
    return filelist;
}

void free_creds() {
    free(username);
    free(password);
}

void file_cleanup() {
    free_filelist();
    free_creds();
}

void file_init() {
    get_local_creds();
}
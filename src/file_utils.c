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

FileList* get_save_filelist() {
    FILE *file = NULL;
    char *line = malloc(MAX_LINE_LENGTH);
    char* split;

    file = fopen(SAVEFILES, "r");

    if(file == NULL)
        return NULL;

    FileList *filelist = malloc(sizeof(FileList));
    FileList *iter = filelist;

    while(fgets(line, MAX_LINE_LENGTH, file)) {
        FileList *new_node = malloc(sizeof(FileList));
        split = strtok(line, " ");
        new_node->name = malloc(strlen(split) + 1);
        strcpy(new_node->name, split);
        split = strtok(NULL, " ");
        new_node->path = malloc(strlen(split) + 1);
        strcpy(new_node->path, strtok(split, "\n"));
        new_node->next = NULL;
        iter->next = new_node;
        iter = new_node;
    }

    // save first node
    iter = filelist;
    // fix first node to point to correct one
    filelist = filelist->next;
    // free unlinked node
    free(iter);

    // cleanup
    fclose(file);
    free(line);

    return filelist;
}

void append_data_to_save_file(char* data) {
    FILE *file = NULL;

    file = fopen(SAVEFILES, "a");

    if(file == NULL)
        exit_msg("failed to open file " SAVEFILES);

    fputs(data, file);

    fclose(file);
}

void free_save_filelist() {
    FileList *cur = filelist;
    FileList *temp;
    while(cur != NULL) {
        temp = cur;
        cur = cur->next;
        free(temp);
    }
}

void get_local_creds() {
    int i = 0;
    FILE *file = NULL;
    char *line = malloc(MAX_LINE_LENGTH);
    size_t len = 0;

    file = fopen(CREDENTIALS_FILE, "r");

    if(file == NULL) {
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
    free_save_filelist(filelist);
    // get the filelist
    filelist = get_save_filelist();
    // return the filelist if it exists
    // otherwise the filelist will be NULL
    return filelist;
}

void free_creds() {
    free(username);
    free(password);
}

void file_cleanup() {
    free_save_filelist(filelist);
    free_creds();
}

void file_init() {
    get_local_creds();
}

#include "global.h"

char* username = NULL;
char* password = NULL;
FileList *file_list = NULL;

char* get_username() {
    return username;
}

char* get_password() {
    return password;
}

FileList* get_filelist() {
    return file_list;
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

    binaryToHex(buffer, binarySize);

    return buffer;
}

FileList* getSavefileList() {
    FILE *file = NULL;
    char *line = malloc(MAX_LINE_LENGTH);

    file = fopen(SAVEFILES, "r");

    if(file == NULL) {
        printf("failed to open file %s\n", SAVEFILES);
        return NULL;
    }

    FileList *file_list = malloc(sizeof(FileList));
    FileList *iter = file_list;

    while(fgets(line, MAX_LINE_LENGTH, file)) {
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

    // cleanup
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

void getLocalCreds() {
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

void freeCreds() {
    free(username);
    free(password);
}

void file_cleanup() {
    freeSavefileList(file_list);
    freeCreds();
}

void file_init() {
    getLocalCreds();
    file_list = getSavefileList();
}

#include "global.h"

FileList *filelist = NULL;
int filelist_size = 0;

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

int check_if_file_exists_in_dir(char *dir, char *file) {
    int creds_file_exists = 0;
    struct dirent **files = NULL;
    int i = scandir(dir, &files, NULL, alphasort);

    if(i < 0)
        exit_msg("scandir failed");

    while(i--) {
        if(strcmp(files[i]->d_name, file) == 0) {
            creds_file_exists = 1;
            break;
        }
    }

    free(files);

    return creds_file_exists;
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
    int save_file_exists = check_if_file_exists_in_dir(".", SAVEFILES);

    if(save_file_exists == 1) {
        FILE *file = NULL;
        file = fopen(SAVEFILES, "r");

        if(file == NULL)
            return NULL;

        char* name;
        char* path;
        FileList *list = malloc(sizeof(FileList));
        FileList *iter = list;
        char *line = malloc(MAX_LINE_LENGTH);
        filelist_size = 0;

        while(fgets(line, MAX_LINE_LENGTH, file)) {
            name = strtok(line, " ");
            path = strtok(NULL, " ");
            path = strtok(path, "\n");
            FileList *new_node = create_filelist_node(path, name);
            iter->next = new_node;
            iter = new_node;
            filelist_size++;
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

    return NULL;
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
        strcat(line, "\n");
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

void write_local_creds() {
    FILE *file = NULL;
    file = fopen(CREDENTIALS_FILE, "w");

    if(file == NULL) {
        printf("failed to write to %s\n", CREDENTIALS_FILE);
        return;
    }

    fputs(username, file);
    fputs("\n", file);
    fputs(password, file);

    fclose(file);
    local_creds_status = LOCAL_CREDS_STATUS_OK;
}

int get_local_creds() {
    int creds_file_exists = check_if_file_exists_in_dir(".", CREDENTIALS_FILE);

    if(creds_file_exists == 1) {
        FILE *file = NULL;
        file = fopen(CREDENTIALS_FILE, "r");

        if(file == NULL) {
            printf("failed to open file %s\n", CREDENTIALS_FILE);
            return LOCAL_CREDS_STATUS_FAILURE;
        }

        int i = 0;
        char *line = malloc(MAX_LINE_LENGTH);
        size_t len = 0;

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
    } else {
        return LOCAL_CREDS_STATUS_FAILURE;
    }

    return LOCAL_CREDS_STATUS_OK;
}

void delete_local_creds() {
    remove(CREDENTIALS_FILE);
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

int get_filelist_size() {
    return filelist_size;
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
    local_creds_status = get_local_creds();
}

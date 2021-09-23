#include "global.h"

SDL_Surface *screen = NULL;
TTF_Font *font = NULL;
char* username = NULL;
char* password = NULL;
int mouse_x = 0;
int mouse_y = 0;
int mouse_pressed = 0;
int mouse_just_pressed = 0;
int local_creds_status = 0;
FileList *filelist = NULL;
int filelist_size = 0;

void exit_msg(char* msg) {
    printf("%s\n", msg);
    exit(-1);
}

#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <json-c/json.h>

#ifdef __3DS__
#include <3ds.h>
#include <malloc.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

#define SDL_FLAGS SDL_DUALSCR
#else
#define SDL_FLAGS SDL_SWSURFACE
#endif

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 480
#define BPP 8

// global variables
extern SDL_Surface *screen;
extern TTF_Font *font;
extern char* username;
extern char* password;
extern int mouse_x;
extern int mouse_y;
extern int mouse_pressed;
extern int mouse_just_pressed;
extern int local_creds_status;

// global functions
void exit_msg(char* msg);

// additional project includes
#include "interface.h"
#include "file_utils.h"
#include "curl_utils.h"
#include "interface_keyboard.h"

extern FileList *filelist;
extern int filelist_size;

#endif

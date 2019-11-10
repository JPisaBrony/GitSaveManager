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

#ifdef _3DS
#include <3ds.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240
#define SDL_FLAGS SDL_DUALSCR
#else
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define SDL_FLAGS SDL_SWSURFACE
#endif

#define BPP 8

// global variables
SDL_Surface *screen;
TTF_Font *font;
char* username;
char* password;

// global functions
void exit_msg(char* msg);

// additional project includes
#include "interface.h"
#include "file_utils.h"
#include "curl_utils.h"
#include "interface_keyboard.h"

#endif

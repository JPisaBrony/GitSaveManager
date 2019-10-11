#include "global.h"

int i, j;
int cur_sel = 0;
int screen_scroll_lower = 0;
int screen_scroll_upper = SCREEN_SCROLL_SIZE;
char *selected_path;
struct dirent **namelist;
struct dirent **original_namelist;
int dir_amount = 0;
int original_dir_amount = 0;
struct stat stat_check;
char *stat_path = NULL;
int current_interface = 0;
SDL_Event event;
SDL_Surface *text = NULL;
TTF_Font *font = NULL;
SDL_Color text_color;
SDL_Color sel_text_color_fg;
SDL_Color sel_text_color_bg;
SDL_Rect text_pos;
SDL_Surface *screen = NULL;

void free_namelist() {
    if(original_namelist != NULL) {
        // free old namelist
        for(i = 0; i < original_dir_amount; i++)
            free(original_namelist[i]);
        free(original_namelist);
    }
}

void cleanup() {
    #ifdef _3DS
    romfsExit();
    socExit();
    #endif
    curl_cleanup();
    file_cleanup();
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);
    free(selected_path);
    free_namelist();
    SDL_Quit();
}

void scan_directory() {
    dir_amount = scandir(selected_path, &namelist, NULL, alphasort);
    // save original pointer and value for freeing later
    original_dir_amount = dir_amount;
    original_namelist = namelist;

    if(dir_amount < 0)
        exit_msg("scandir failed");
    dir_amount--;

    int inc_amount = 0;
    if(namelist != NULL) {
        // check index 0 if it contains either . or ..
        if(strcmp(namelist[0]->d_name, ".") == 0 || strcmp(namelist[0]->d_name, "..") == 0) {
            inc_amount++;
            namelist++;
        }
        // check index 0 again if the namelist pointer was increamented to get both . or ..
        if(strcmp(namelist[0]->d_name, ".") == 0 || strcmp(namelist[0]->d_name, "..") == 0) {
            inc_amount++;
            namelist++;
        }
    }

    dir_amount -= inc_amount;

    if(screen_scroll_upper > dir_amount)
        screen_scroll_upper = dir_amount + 1;
}

void reset_selected_path() {
    if(selected_path != NULL) {
        selected_path[0] = '/';
        selected_path[1] = '\0';
    }
}

void reset_scroll_vars() {
    cur_sel = 0;
    screen_scroll_lower = 0;
    screen_scroll_upper = SCREEN_SCROLL_SIZE;
    if(screen_scroll_upper > dir_amount)
        screen_scroll_upper = dir_amount + 1;
}

void render_text(char* text, int x, int y) {
    // set text position
    text_pos.x = x;
    text_pos.y = y;
    // create text with the font and color
    text = TTF_RenderText_Solid(font, text, text_color);
    // blit the text to the screen
    SDL_BlitSurface(text, NULL, screen, &text_pos);
    // free the text to prevent memory leaks
    SDL_FreeSurface(text);
}

void main_screen_keyboard() {
    switch(event.key.keysym.sym) {
        // quit
        case SDLK_ESCAPE:
        case 'q':
            cleanup();
            exit(0);
        case 'x':
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'z':
            current_interface = SELECTION_SCREEN;
            reset_selected_path();
            scan_directory();
            reset_scroll_vars();
            break;
        default:
            break;
    }
}

void main_screen_render() {
    render_text("Git Save Manager", 0, 0);
    render_text("Press Y (3DS) or Z (Desktop) to add files to be managed", 0, TEXT_HEIGHT * 2);
    render_text("Press X to view managed files", 0, TEXT_HEIGHT * 3);
    render_text("Press Select (3DS) or q (Desktop) to quit", 0, TEXT_HEIGHT * 4);
}

void managed_files_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 's':
            current_interface = MAIN_SCREEN;
            break;
        default:
            break;
    }
}

void managed_files_screen_render() {
    render_text("Managed Files", 0, 0);

    FileList *files = get_filelist();

    int i = TEXT_HEIGHT * 2;
    char* full_text = malloc(1024);
    FileList *cur = files;
    while(cur != NULL) {
        sprintf(full_text, "%s : %s", cur->name, cur->path);
        render_text(full_text, 0, i);
        cur = cur->next;
        i += TEXT_HEIGHT;
        // make sure to render on the screen
        if(i / TEXT_HEIGHT > SCREEN_SCROLL_SIZE - 2)
            break;
    }
    free(full_text);
}

void selection_confirm_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 's':
            current_interface = SELECTION_SCREEN;
            break;
        case 'a':
            current_interface = MAIN_SCREEN;
            break;
        default:
            break;
    }
}

void selection_confirm_screen_render() {
    render_text("Manage this file?", 0, 0);
    render_text(namelist[cur_sel]->d_name, 0, TEXT_HEIGHT * 2);
    render_text("Yes - Press A", 0, TEXT_HEIGHT * 4);
    render_text("No - Press B", 0, TEXT_HEIGHT * 5);
}

void selection_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'q':
            current_interface = MAIN_SCREEN;
            break;
        case SDLK_b:
        case SDLK_BACKSPACE:
            j = 0;
            for(i = strlen(selected_path); i >= 0; i--) {
                if(selected_path[i] == '/') {
                    j++;
                }

                if(j == 2) {
                    selected_path[i+1] = '\0';
                    break;
                }
            }

            free_namelist();
            scan_directory();
            reset_scroll_vars();
            break;
        case SDLK_a:
        case SDLK_RETURN:
            if(dir_amount > -1) {
                stat_path = malloc((strlen(selected_path) * sizeof(char*)) + (strlen(namelist[cur_sel]->d_name) * sizeof(char*)));
                strcpy(stat_path, selected_path);
                strcat(stat_path, namelist[cur_sel]->d_name);
                stat(stat_path, &stat_check);
                free(stat_path);
                if(!S_ISREG(stat_check.st_mode)) {
                    strcat(selected_path, namelist[cur_sel]->d_name);
                    strcat(selected_path, "/");

                    free_namelist();
                    scan_directory();
                    reset_scroll_vars();
                } else {
                    current_interface = SELECTION_CONFIRM_SCREEN;
                }
            }
            break;
        case SDLK_UP:
            if(cur_sel > 0)
                cur_sel--;
            break;
        case SDLK_DOWN:
            if(cur_sel < dir_amount)
                cur_sel++;
            break;
        default:
            break;
    }
}

void selection_screen_render() {
    if(cur_sel >= screen_scroll_upper && cur_sel <= dir_amount) {
        screen_scroll_lower++;
        screen_scroll_upper++;
    } else if(cur_sel < screen_scroll_lower && cur_sel > -1) {
        screen_scroll_lower--;
        screen_scroll_upper--;
    }

    if(namelist != NULL) {
        j = 0;
        for(i = screen_scroll_lower; i < screen_scroll_upper; i++) {
            // update text pos
            text_pos.y = j * TEXT_HEIGHT;
            if(cur_sel == i) {
                // create text with font and selected color
                text = TTF_RenderText_Shaded(font, namelist[i]->d_name, sel_text_color_fg, sel_text_color_bg);
            } else {
                // create text with the font and color
                text = TTF_RenderText_Solid(font, namelist[i]->d_name, text_color);
            }
            // blit the text to the screen
            SDL_BlitSurface(text, NULL, screen, &text_pos);
            // free the text to prevent memory leaks
            SDL_FreeSurface(text);
            j++;
        }
    }
}

void interface_init() {
    if(SDL_Init(SDL_INIT_VIDEO) == -1)
         exit_msg("Couldn't init SDL");

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BPP, SDL_FLAGS);

    if(screen == NULL)
        exit_msg("Couldn't init SDL Window");

    if(TTF_Init() == -1)
        exit_msg("Couldn't init SDL TTF");

    #ifdef _3DS
    SDL_ShowCursor(SDL_DISABLE);
    romfsInit();
    font = TTF_OpenFont("romfs:/FreeMonoBold.ttf", FONT_SIZE);
    #else
    font = TTF_OpenFont("FreeMonoBold.ttf", FONT_SIZE);
    #endif

    if(font == NULL)
        exit_msg("Failed to open font");

    // set text position
    text_pos.x = 0;
    text_pos.y = 0;

    // set text color to white
    text_color.r = 0xFF;
    text_color.g = 0xFF;
    text_color.b = 0xFF;

    // selected text color
    sel_text_color_bg.r = 0x00;
    sel_text_color_bg.g = 0x37;
    sel_text_color_bg.b = 0xFF;

    sel_text_color_fg.r = 0xFF;
    sel_text_color_fg.g = 0xFF;
    sel_text_color_fg.b = 0xFF;

    selected_path = malloc(1024);
    reset_selected_path();

    #ifdef _3DS
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
    #endif
}

void main_interface() {
    while(1) {
        // check for pending events
        while(SDL_PollEvent(&event)) {
            // quit was requested
            if(event.type == SDL_QUIT) {
                cleanup();
                exit(0);
            // keyboard button was hit
            } else if (event.type == SDL_KEYDOWN) {
                switch(current_interface) {
                    case MAIN_SCREEN:
                        main_screen_keyboard();
                        break;
                    case MANAGED_FILE_SCREEN:
                        managed_files_screen_keyboard();
                        break;
                    case SELECTION_SCREEN:
                        selection_screen_keyboard();
                        break;
                    case SELECTION_CONFIRM_SCREEN:
                        selection_confirm_screen_keyboard();
                        break;
                }
            }
        }
        // clear screen
        SDL_FillRect(screen, NULL, 0x00000000);

        switch(current_interface) {
            case MAIN_SCREEN:
                main_screen_render();
                break;
            case MANAGED_FILE_SCREEN:
                managed_files_screen_render();
                break;
            case SELECTION_SCREEN:
                selection_screen_render();
                break;
            case SELECTION_CONFIRM_SCREEN:
                selection_confirm_screen_render();
                break;
        }

        if(SDL_Flip(screen) == -1)
            exit_msg("Failed to SDL_Flip");
    }
}

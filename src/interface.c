#include "global.h"

int i, j;
int cur_sel = 0;
int screen_scroll_lower = 0;
int screen_scroll_upper = SCREEN_SCROLL_SIZE;
int held_delay = -1;
char *selected_path = NULL;
struct dirent **namelist = NULL;
struct dirent **original_namelist = NULL;
int dir_amount = 0;
int original_dir_amount = 0;
struct stat stat_check;
char *stat_path = NULL;
int current_interface = 0;
FileList *files = NULL;
FileList *cur_file = NULL;
int files_size = 0;
SDL_Event event;
SDL_Surface *text = NULL;
SDL_Color text_color;
SDL_Color sel_text_color_fg;
SDL_Color sel_text_color_bg;
SDL_Rect text_pos;

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

void screen_keyboard_held_up_or_down(int upper_limit) {
    if(held_delay >= HELD_DELAY) {
        held_delay = 0;
    } else if(held_delay != -1) {
        held_delay++;
    }

    const Uint8 *key_state = SDL_GetKeyState(NULL);

    if(key_state[SDLK_UP] && cur_sel > 0) {
        if(held_delay == -1)
            held_delay = 0;
        if(held_delay == 0)
            cur_sel--;
    } else if(key_state[SDLK_DOWN] && cur_sel < upper_limit) {
        if(held_delay == -1)
            held_delay = 0;
        if(held_delay == 0)
            cur_sel++;
    } else {
        held_delay = -1;
    }
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

void reset_managed_screen_scroll_vars() {
    files = get_filelist();
    files_size = get_filelist_size() - 1;
    cur_sel = 0;
    screen_scroll_lower = 0;
    screen_scroll_upper = SCREEN_SCROLL_SIZE - 2;
}

void render_text(char* msg) {
    // create text with the font and color
    text = TTF_RenderText_Solid(font, msg, text_color);
    // blit the text to the screen
    SDL_BlitSurface(text, NULL, screen, &text_pos);
    // free the text to prevent memory leaks
    SDL_FreeSurface(text);
}

void render_color_text(char* msg) {
    // create text with the font and color
    text = TTF_RenderText_Shaded(font, msg, sel_text_color_fg, sel_text_color_bg);
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
            reset_managed_screen_scroll_vars();
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'y':
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
    text_pos.y = 0;
    render_text("Git Save Manager");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text("Press Y (3DS) or Z (Desktop) to add files to be managed");
    text_pos.y = TEXT_HEIGHT * 3;
    render_text("Press X to view managed files");
    text_pos.y = TEXT_HEIGHT * 4;
    render_text("Press Select (3DS) or q (Desktop) to quit");
}

void find_current_file_node() {
    i = 0;
    FileList *cur = files;

    while(cur != NULL) {
        if(i == cur_sel) {
            cur_file = cur;
            break;
        }
        cur = cur->next;
        i++;
    }
}

void file_manage_screen_keyboard() {
    char *url = NULL;
    switch(event.key.keysym.sym) {
        case 'a':
            url = getUrlFromGistByFilename(cur_file->name);
            if(url == NULL)
                createGist(cur_file->path, cur_file->name);
            else
                updateGist(cur_file->name, cur_file->path, url);
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'y':
            url = getUrlFromGistByFilename(cur_file->name);
            if(url != NULL)
                getAndSaveGist(cur_file->path, url);
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'x':
            delete_node_from_filelist(&files, cur_sel);
            write_save_file_from_filelist(files);
            reset_managed_screen_scroll_vars();
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'b':
            current_interface = MANAGED_FILE_SCREEN;
            break;
        default:
            break;
    }
    free(url);
}

void file_manage_screen_render() {
    text_pos.y = 0;
    render_text("File Manage");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text(cur_file->name);
    text_pos.y = TEXT_HEIGHT * 3;
    render_text(cur_file->path);
    text_pos.y = TEXT_HEIGHT * 5;
    render_text("Upload - Press A");
    text_pos.y = TEXT_HEIGHT * 6;
    render_text("Download - Press Y");
    text_pos.y = TEXT_HEIGHT * 7;
    render_text("Delete - Press X");
    text_pos.y = TEXT_HEIGHT * 8;
    render_text("Cancel - Press B");
}

void managed_files_screen_keyboard_held() {
    screen_keyboard_held_up_or_down(files_size);
}

void managed_files_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'b':
        case 's':
            current_interface = MAIN_SCREEN;
            break;
        case 'a':
        case SDLK_RETURN:
            find_current_file_node();
            current_interface = FILE_MANAGE_SCREEN;
            break;
        default:
            break;
    }
}

void managed_files_screen_render() {
    text_pos.y = 0;
    render_text("Managed Files");

    i = TEXT_HEIGHT * 2;
    text_pos.y = TEXT_HEIGHT * 2;

    if(cur_sel >= screen_scroll_upper && cur_sel < files_size) {
        screen_scroll_lower++;
        screen_scroll_upper++;
    } else if(cur_sel < screen_scroll_lower && cur_sel > -1) {
        screen_scroll_lower--;
        screen_scroll_upper--;
    }

    if(files == NULL) {
        render_text("No currently managed files");
    } else {
        char* full_text = malloc(MAX_LINE_LENGTH);
        FileList *cur = files;
        j = 0;
        while(cur != NULL) {
            if(j >= screen_scroll_lower && j <= screen_scroll_upper) {
                sprintf(full_text, "%s : %s", cur->name, cur->path);
                if(cur_sel == j) {
                    render_color_text(full_text);
                } else {
                    render_text(full_text);
                }
                text_pos.y += TEXT_HEIGHT;
            }
            j++;
            cur = cur->next;
        }
        free(full_text);
    }
}

void selection_confirm_screen_keyboard() {
    FileList *node;
    switch(event.key.keysym.sym) {
        case 'b':
        case 's':
            current_interface = SELECTION_SCREEN;
            break;
        case 'a':
            node = create_filelist_node(selected_path, namelist[cur_sel]->d_name);
            append_node_to_save_file(node);
            free_filelist_node(node);
            current_interface = MAIN_SCREEN;
            break;
        default:
            break;
    }
}

void selection_confirm_screen_render() {
    text_pos.y = 0;
    render_text("Manage this file?");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text(namelist[cur_sel]->d_name);
    text_pos.y = TEXT_HEIGHT * 4;
    render_text("Yes - Press A");
    text_pos.y = TEXT_HEIGHT * 5;
    render_text("No - Press B");
}

void selection_screen_keyboard_held() {
    screen_keyboard_held_up_or_down(dir_amount);
}

void selection_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'y':
        case 'q':
            current_interface = MAIN_SCREEN;
            break;
        case 'b':
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
        case 'a':
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
        default:
            break;
    }
}

void selection_screen_render() {
    if(cur_sel >= screen_scroll_upper - 1 && cur_sel < dir_amount) {
        screen_scroll_lower++;
        screen_scroll_upper++;
    } else if(cur_sel < screen_scroll_lower && cur_sel > -1) {
        screen_scroll_lower--;
        screen_scroll_upper--;
    }

    if(namelist != NULL) {
        j = 0;
        for(i = screen_scroll_lower; i < screen_scroll_upper; i++) {
            text_pos.y = j * TEXT_HEIGHT;
            if(cur_sel == i) {
                render_color_text(namelist[i]->d_name);
            } else {
                render_text(namelist[i]->d_name);
            }
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
    if(SOC_buffer == NULL)
        exit_msg("memalign: failed to allocate\n");

    // Now intialise soc:u service
    int ret;
    if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0)
        exit_msg("socInit failed\n");

    #endif
}

void main_interface() {
    while(1) {
        // check for keys that have repeated actions for being held
        switch(current_interface) {
            case MANAGED_FILE_SCREEN:
                managed_files_screen_keyboard_held();
                break;
            case SELECTION_SCREEN:
                selection_screen_keyboard_held();
                break;
        }

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
                    case FILE_MANAGE_SCREEN:
                        file_manage_screen_keyboard();
                        break;
                }
            } else if(event.type == SDL_MOUSEBUTTONDOWN) {
                mouse_pressed = 1;
                mouse_just_pressed = 1;
                SDL_GetMouseState(&mouse_x, &mouse_y);
            } else if(event.type == SDL_MOUSEBUTTONUP) {
                mouse_pressed = 0;
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
            case FILE_MANAGE_SCREEN:
                file_manage_screen_render();
                break;
        }

        if(SDL_Flip(screen) == -1)
            exit_msg("Failed to SDL_Flip");
    }
}

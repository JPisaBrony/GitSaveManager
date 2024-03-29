#include "global.h"

int i, j;
int cur_sel = 0;
int screen_scroll_lower = 0;
int screen_scroll_upper = SCREEN_SCROLL_SIZE;
int held_delay = -1;
char *selected_path = NULL;
char *current_name = NULL;
struct dirent **namelist = NULL;
struct dirent **original_namelist = NULL;
int dir_amount = 0;
int original_dir_amount = 0;
struct stat stat_check;
char *stat_path = NULL;
int current_interface = 0;
FileList *cur_file = NULL;
SDL_Event event;
SDL_Surface *text = NULL;
SDL_Color text_color;
SDL_Color sel_text_color_fg;
SDL_Color sel_text_color_bg;
SDL_Rect text_pos;
int github_input_location = 0;

void free_namelist() {
    if(original_namelist != NULL) {
        // free old namelist
        for(i = 0; i < original_dir_amount; i++)
            free(original_namelist[i]);
        free(original_namelist);
    }
}

void cleanup() {
    #ifdef __3DS__
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
    get_filelist();
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

void github_creds_password_enter_func() {
    if(strcmp("password", "") != 0) {
        write_local_creds();
        keyboard_clear_enter_func();
        current_interface = MAIN_SCREEN;
    }
}

void github_creds_username_enter_func() {
    if(strcmp("username", "") != 0) {
        github_input_location = TEXT_HEIGHT * 6;
        keyboard_clear_enter_func();
        keyboard_setup(password, &github_creds_password_enter_func);
    }
}

void github_setup_screen() {
    if(username == NULL) {
        username = malloc(MAX_INPUT_LENGTH);
        username[0] = '\0';
    }
    if(password == NULL) {
        password = malloc(MAX_INPUT_LENGTH);
        password[0] = '\0';
    }
    github_input_location = TEXT_HEIGHT * 3;
    keyboard_setup(username, &github_creds_username_enter_func);
    current_interface = GITHUB_CREDS_SCREEN;
}

void main_screen_keyboard() {
    switch(event.key.keysym.sym) {
        // quit
        case SDLK_ESCAPE:
            cleanup();
            exit(0);
        case 'x':
            reset_managed_screen_scroll_vars();
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'y':
            reset_selected_path();
            scan_directory();
            reset_scroll_vars();
            current_interface = SELECTION_SCREEN;
            break;
        case SDLK_RETURN:
            if(local_creds_status == LOCAL_CREDS_STATUS_OK) {
                current_interface = GITHUB_RESET_CREDS_SCREEN;
            } else {
                github_setup_screen();
            }
            break;
        default:
            break;
    }
}

void main_screen_render() {
    text_pos.y = 0;
    render_text("Git Save Manager");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text("Press Y to add files to be managed");
    text_pos.y = TEXT_HEIGHT * 3;
    render_text("Press X to view managed files");
    text_pos.y = TEXT_HEIGHT * 4;
    render_text("Press Start to login to Github");
    text_pos.y = TEXT_HEIGHT * 5;
    render_text("Press Select to quit");
}

void find_current_file_node() {
    i = 0;
    FileList *cur = filelist;

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
            if(url == NULL) {
                if(createGist(cur_file->path, cur_file->name) == -1) {
                    current_interface = INVALID_CREDS_SCREEN;
                } else {
                    current_interface = MANAGED_FILE_SCREEN;
                }
            } else if(url[0] == 'e') {
                current_interface = INVALID_CREDS_SCREEN;
            } else {
                if(updateGist(cur_file->name, cur_file->path, url) == -1) {
                    current_interface = INVALID_CREDS_SCREEN;
                } else {
                    current_interface = MANAGED_FILE_SCREEN;
                }
                free(url);
            }
            break;
        case 'y':
            url = getUrlFromGistByFilename(cur_file->name);
            if(url != NULL) {
                if(url[0] == 'e') {
                    current_interface = INVALID_CREDS_SCREEN;
                } else {
                    if(getAndSaveGist(cur_file->path, url) == -1) {
                        current_interface = INVALID_CREDS_SCREEN;
                    } else {
                        current_interface = MANAGED_FILE_SCREEN;
                    }
                    free(url);
                }
            } else {
                current_interface = MANAGED_FILE_SCREEN;
            }
            break;
        case 'x':
            delete_node_from_filelist(&filelist, cur_sel);
            write_save_file_from_filelist(filelist);
            reset_managed_screen_scroll_vars();
            current_interface = MANAGED_FILE_SCREEN;
            break;
        case 'b':
            current_interface = MANAGED_FILE_SCREEN;
            break;
        default:
            break;
    }
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
    screen_keyboard_held_up_or_down(filelist_size);
}

void managed_files_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'b':
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

    if(cur_sel >= screen_scroll_upper && cur_sel < filelist_size) {
        screen_scroll_lower++;
        screen_scroll_upper++;
    } else if(cur_sel < screen_scroll_lower && cur_sel > -1) {
        screen_scroll_lower--;
        screen_scroll_upper--;
    }

    if(filelist == NULL) {
        render_text("No currently managed files");
    } else {
        char* full_text = malloc(MAX_LINE_LENGTH);
        FileList *cur = filelist;
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

void selection_confirm_screen_enter_func() {
    // make sure string is not empty before going back
    if(strcmp(current_name, "") != 0) {
        keyboard_clear_enter_func();
        current_interface = SELECTION_CONFIRM_SCREEN;
    }
}

void selection_confirm_screen_keyboard() {
    FileList *node;
    char *full_path;
    switch(event.key.keysym.sym) {
        case 'b':
            free(current_name);
            current_interface = SELECTION_SCREEN;
            break;
        case 'a':
            full_path = malloc((strlen(selected_path) * sizeof(char*)) + (strlen(namelist[cur_sel]->d_name) * sizeof(char*)));
            strcpy(full_path, selected_path);
            strcat(full_path, namelist[cur_sel]->d_name);
            node = create_filelist_node(full_path, current_name);
            append_node_to_save_file(node);
            free_filelist_node(node);
            free(current_name);
            free(full_path);
            current_interface = MAIN_SCREEN;
            break;
        case 'y':
            keyboard_setup(current_name, &selection_confirm_screen_enter_func);
            current_interface = SELECTION_RENAME_SCREEN;
            break;
        default:
            break;
    }
}

void selection_confirm_screen_render() {
    text_pos.y = 0;
    render_text("Manage this file?");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text(current_name);
    text_pos.y = TEXT_HEIGHT * 4;
    render_text("Yes - Press A");
    text_pos.y = TEXT_HEIGHT * 5;
    render_text("No - Press B");
    text_pos.y = TEXT_HEIGHT * 6;
    render_text("Rename - Press Y");
}

void selection_screen_keyboard_held() {
    screen_keyboard_held_up_or_down(dir_amount);
}

void selection_rename_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'b':
            selection_confirm_screen_enter_func();
            break;
        default:
            break;
    }
}

void selection_rename_screen_render() {
    text_pos.y = 0;
    render_text("Rename file");
    text_pos.y = TEXT_HEIGHT * 4;
    render_text("Done - Press B");
    show_keyboard(0, TEXT_HEIGHT * 2);
}

void selection_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case SDLK_ESCAPE:
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
                    current_name = malloc(strlen(namelist[cur_sel]->d_name) * sizeof(char*));
                    strcpy(current_name, namelist[cur_sel]->d_name);
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

void invalid_creds_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'b':
            current_interface = FILE_MANAGE_SCREEN;
        default:
            break;
    }
}

void invalid_creds_screen_render() {
    text_pos.y = 0;
    render_text("Invalid Credentials");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text("Back - Press B");
}

void github_creds_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'b':
            if(local_creds_status == LOCAL_CREDS_STATUS_FAILURE) {
                username[0] = '\0';
                password[0] = '\0';
            }
            current_interface = MAIN_SCREEN;
        default:
            break;
    }
}

void github_creds_screen_render() {
    text_pos.y = 0;
    render_text("Github Login");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text("Username");
    if(github_input_location != TEXT_HEIGHT * 3) {
        text_pos.y = TEXT_HEIGHT * 3;
        render_text(username);
    }
    text_pos.y = TEXT_HEIGHT * 5;
    render_text("Password");
    text_pos.y = TEXT_HEIGHT * 8;
    render_text("Cancel - Press B");
    show_keyboard(0, github_input_location);
}

void github_reset_creds_screen_keyboard() {
    switch(event.key.keysym.sym) {
        case 'a':
            free(username);
            username = NULL;
            free(password);
            password = NULL;
            delete_local_creds();
            local_creds_status = LOCAL_CREDS_STATUS_FAILURE;
            github_setup_screen();
            current_interface = GITHUB_CREDS_SCREEN;
            break;
        case 'b':
            current_interface = MAIN_SCREEN;
        default:
            break;
    }
}

void github_reset_creds_screen_render() {
    text_pos.y = 0;
    render_text("Reset Github Credentials?");
    text_pos.y = TEXT_HEIGHT * 2;
    render_text("Yes - Press A");
    text_pos.y = TEXT_HEIGHT * 3;
    render_text("No - Press B");
}

void interface_init() {
    if(SDL_Init(SDL_INIT_VIDEO) == -1)
         exit_msg("Couldn't init SDL");

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BPP, SDL_FLAGS);

    if(screen == NULL)
        exit_msg("Couldn't init SDL Window");

    if(TTF_Init() == -1)
        exit_msg("Couldn't init SDL TTF");

    #ifdef __3DS__
    SDL_ShowCursor(SDL_DISABLE);
    romfsInit();
    font = TTF_OpenFont("romfs:FreeMonoBold.ttf", FONT_SIZE);
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

    #ifdef __3DS__
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
                    case SELECTION_RENAME_SCREEN:
                        selection_rename_screen_keyboard();
                        break;
                    case FILE_MANAGE_SCREEN:
                        file_manage_screen_keyboard();
                        break;
                    case GITHUB_CREDS_SCREEN:
                        github_creds_screen_keyboard();
                        break;
                    case GITHUB_RESET_CREDS_SCREEN:
                        github_reset_creds_screen_keyboard();
                        break;
                    case INVALID_CREDS_SCREEN:
                        invalid_creds_screen_keyboard();
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
            case SELECTION_RENAME_SCREEN:
                selection_rename_screen_render();
                break;
            case FILE_MANAGE_SCREEN:
                file_manage_screen_render();
                break;
            case GITHUB_CREDS_SCREEN:
                github_creds_screen_render();
                break;
            case GITHUB_RESET_CREDS_SCREEN:
                github_reset_creds_screen_render();
                break;
            case INVALID_CREDS_SCREEN:
                invalid_creds_screen_render();
                break;
        }

        if(SDL_Flip(screen) == -1)
            exit_msg("Failed to SDL_Flip");
    }
}

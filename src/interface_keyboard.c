#include "global.h"

int i, j;
int mouse_x = 0;
int mouse_y = 0;
int mouse_pressed = 0;
int mouse_just_pressed = 0;
SDL_Rect rect;
SDL_Event e;
SDL_Color text_color_fg;
Uint32 text_color_bg = 0;
Uint32 text_color_bg_pressed = 0;
int key_state = KEYS_LOWER;
int shift_pressed = 0;

char *input;
char **keys;

char *keys_lower[KEYS_ROWS] =
{"1234567890",
 "qwertyuiop",
 "asdfghjkl",
 "zxcvbnm"
};

char *keys_upper[KEYS_ROWS] =
{"!@#$%^&*()",
 "QWERTYUIOP",
 "ASDFGHJKL",
 "ZXCVBNM"
};

void key_button(char letter) {
    char msg[2];
    msg[0] = letter;
    msg[1] = '\0';
    // create text with the font and color
    SDL_Surface *text = TTF_RenderText_Solid(font, msg, text_color_fg);
    // background rectangle bounds
    rect.x = rect.x - KEYBOARD_KEY_BACKGROUND_SPACE;
    rect.w = (KEYBOARD_X_INCREMENT - KEYBOARD_KEY_BACKGROUND_SPACE);
    rect.h = KEYBOARD_X_INCREMENT;
    // check if mouse is inside the rectangle
    if(mouse_x > rect.x && mouse_x < rect.x + rect.w && mouse_y > rect.y && mouse_y < rect.y + rect.h && mouse_pressed == 1) {
        SDL_FillRect(screen, &rect, text_color_bg_pressed);
        if(mouse_just_pressed == 1) {
            strcat(input, msg);
            mouse_just_pressed = 0;
        }
    } else {
        // render background rectangle
        SDL_FillRect(screen, &rect, text_color_bg);
    }
    // reset text x pos
    rect.x = rect.x + KEYBOARD_KEY_BACKGROUND_SPACE;
    // blit the text to the screen
    SDL_BlitSurface(text, NULL, screen, &rect);
    // free the text to prevent memory leaks
    SDL_FreeSurface(text);


}

void special_key_button(char *msg, int action) {
    // create text with the font and color
    SDL_Surface *text = TTF_RenderText_Solid(font, msg, text_color_fg);

    // background rectangle bounds
    int len = strlen(msg) - 1;
    rect.x = rect.x - KEYBOARD_KEY_BACKGROUND_SPACE;
    rect.w = len * (KEYBOARD_X_INCREMENT - KEYBOARD_KEY_BACKGROUND_SPACE);

    // check if mouse is inside the rectangle
    if(mouse_x > rect.x && mouse_x < rect.x + rect.w && mouse_y > rect.y && mouse_y < rect.y + rect.h && mouse_pressed == 1) {
        SDL_FillRect(screen, &rect, text_color_bg_pressed);
        switch(action) {
            case SPECIAL_KEY_ACTION_BACKSPACE:
                if(mouse_just_pressed == 1) {
                    len = strlen(input);
                    input[len - 1] = '\0';
                    mouse_just_pressed = 0;
                }
                break;
            case SPECIAL_KEY_ACTION_ENTER:
                break;
            case SPECIAL_KEY_ACTION_CAPS:
                if(mouse_just_pressed == 1) {
                    if(key_state == KEYS_LOWER) {
                        keys = keys_upper;
                        key_state = KEYS_UPPER;
                    } else if(key_state == KEYS_UPPER) {
                        keys = keys_lower;
                        key_state = KEYS_LOWER;
                    }
                    mouse_just_pressed = 0;
                }
                break;
            case SPECIAL_KEY_ACTION_SHIFT:
                if(mouse_just_pressed == 1) {
                    if(key_state == KEYS_LOWER) {
                        keys = keys_upper;
                        key_state = KEYS_UPPER;
                    } else if(key_state == KEYS_UPPER) {
                        keys = keys_lower;
                        key_state = KEYS_LOWER;
                    }
                    mouse_just_pressed = 0;
                    shift_pressed = 1;
                }
                break;
            default:
                break;
        }
    } else {
        // render background rectangle
        SDL_FillRect(screen, &rect, text_color_bg);
    }

    // reset text x pos
    rect.x = rect.x + KEYBOARD_KEY_BACKGROUND_SPACE;
    // blit the text to the screen
    SDL_BlitSurface(text, NULL, screen, &rect);
    // free the text to prevent memory leaks
    SDL_FreeSurface(text);
}

void keyboard_init() {
    // text color
    text_color_fg.r = 0xFF;
    text_color_fg.g = 0xFF;
    text_color_fg.b = 0xFF;
    text_color_bg = SDL_MapRGB(screen->format, 0x00, 0x37, 0xFF);
    text_color_bg_pressed = SDL_MapRGB(screen->format, 0x03, 0xF8, 0xFC);

    input = malloc(MAX_INPUT_LENGTH);
    input[0] = '\0';

    // setup keys
    keys = keys_lower;

    while(1) {
        // check for pending events
        while(SDL_PollEvent(&e)) {
            // quit was requested
            if(e.type == SDL_QUIT) {
                exit(0);
            } else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case 'q':
                        SDL_Quit();
                        exit(0);
                        break;
                    default:
                        break;
                }
            } else if(e.type == SDL_MOUSEBUTTONDOWN) {
                mouse_pressed = 1;
                mouse_just_pressed = 1;
                SDL_GetMouseState(&mouse_x, &mouse_y);
            } else if(e.type == SDL_MOUSEBUTTONUP) {
                mouse_pressed = 0;
            }
        }

        // clear screen
        SDL_FillRect(screen, NULL, 0x00000000);

        // print input to top of screen
        rect.x = 0;
        rect.y = 0;
        // create text with the font and color
        SDL_Surface *text = TTF_RenderText_Solid(font, input, text_color_fg);
        // blit the text to the screen
        SDL_BlitSurface(text, NULL, screen, &rect);
        // free the text to prevent memory leaks
        SDL_FreeSurface(text);

        for(i = 0; i < KEYS_ROWS; i++) {
            rect.y = KEYBOARD_START_Y + (i * KEYBOARD_Y_INCREMENT) + KEYBOARD_KEY_SPACING;
            int len = strlen(keys[i]);
            for(j = 0; j < len; j++) {
                rect.x = KEYBOARD_START_X + (j * KEYBOARD_X_INCREMENT) + KEYBOARD_KEY_SPACING + i * KEYBOARD_KEY_SPACING;
                key_button(keys[i][j]);
            }
        }

        int len = strlen(keys[0]);
        rect.x = KEYBOARD_START_X + len * KEYBOARD_X_INCREMENT + KEYBOARD_KEY_SPACING;
        rect.y = KEYBOARD_START_Y + KEYBOARD_KEY_SPACING;
        special_key_button("<==", SPECIAL_KEY_ACTION_BACKSPACE);

        len = strlen(keys[2]);
        rect.x = KEYBOARD_START_X + (len + 1) * KEYBOARD_X_INCREMENT + KEYBOARD_KEY_SPACING - KEYBOARD_KEY_BACKGROUND_SPACE;
        rect.y = KEYBOARD_START_Y + KEYBOARD_Y_INCREMENT * 2 + KEYBOARD_KEY_SPACING;
        special_key_button("==>", SPECIAL_KEY_ACTION_ENTER);

        //len = strlen(keys[3]);
        rect.x = KEYBOARD_START_X - KEYBOARD_KEY_BACKGROUND_SPACE;
        rect.y = KEYBOARD_START_Y + KEYBOARD_Y_INCREMENT * 3 + KEYBOARD_KEY_SPACING;
        special_key_button("[s]", SPECIAL_KEY_ACTION_SHIFT);

        if(SDL_Flip(screen) == -1)
            exit_msg("Failed to SDL_Flip");
    }
}

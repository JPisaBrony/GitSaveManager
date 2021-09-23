#include "global.h"

SDL_Rect rect;
SDL_Color text_color_fg;
Uint32 text_color_bg = 0;
Uint32 text_color_bg_pressed = 0;
int key_state = KEYS_LOWER;
int shift_pressed = 0;
void (*enter_func)() = NULL;

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

char *keys_extra[KEYS_ROWS] =
{"-=_+      ",
 "[]\{}|     ",
 ";\':\"     ",
 ",./<>? "
};

void switch_letter_case() {
    if(key_state == KEYS_LOWER) {
        keys = keys_upper;
        key_state = KEYS_UPPER;
    } else if(key_state == KEYS_UPPER) {
        keys = keys_lower;
        key_state = KEYS_LOWER;
    }
}

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
            if(shift_pressed == 1) {
                switch_letter_case();
                shift_pressed = 0;
            }
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
    int len = 0;
    if(strlen(msg) > 1)
        len = strlen(msg) - 1;
    else
        len = 1;

    rect.x = rect.x - KEYBOARD_KEY_BACKGROUND_SPACE;
    rect.w = len * (KEYBOARD_X_INCREMENT - KEYBOARD_KEY_BACKGROUND_SPACE);

    // check if mouse is inside the rectangle
    if(mouse_x > rect.x && mouse_x < rect.x + rect.w && mouse_y > rect.y && mouse_y < rect.y + rect.h && mouse_pressed == 1) {
        SDL_FillRect(screen, &rect, text_color_bg_pressed);
        if(mouse_just_pressed == 1) {
            switch(action) {
                case SPECIAL_KEY_ACTION_BACKSPACE:
                    len = strlen(input);
                    input[len - 1] = '\0';
                    break;
                case SPECIAL_KEY_ACTION_ENTER:
                    if(enter_func != NULL)
                        enter_func();
                    break;
                case SPECIAL_KEY_ACTION_CAPS:
                    if(key_state != KEYS_EXTRA) {
                        switch_letter_case();
                    }
                    break;
                case SPECIAL_KEY_ACTION_SHIFT:
                    if(key_state != KEYS_EXTRA) {
                        switch_letter_case();
                        shift_pressed = !shift_pressed;
                    }
                    break;
                case SPECIAL_KEY_ACTION_SPACE:
                    strcat(input, " ");
                    break;
                case SPECIAL_KEY_ACTION_BACKQUOTE:
                    strcat(input, "`");
                    break;
                case SPECIAL_KEY_ACTION_TILDE:
                    if(shift_pressed == 1) {
                        switch_letter_case();
                        shift_pressed = 0;
                    }
                    strcat(input, "~");
                    break;
                case SPECIAL_KEY_ACTION_EXTRA:
                    if(key_state != KEYS_EXTRA) {
                        keys = keys_extra;
                        key_state = KEYS_EXTRA;
                        shift_pressed = 0;
                    } else {
                        keys = keys_lower;
                        key_state = KEYS_LOWER;
                    }
                    break;
                case SPECIAL_KEY_ACTION_CLEAR:
                    input[0] = '\0';
                    break;
                default:
                    break;
            }
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

void keyboard_init() {
    // setup text color
    text_color_fg.r = 0xFF;
    text_color_fg.g = 0xFF;
    text_color_fg.b = 0xFF;
    text_color_bg = SDL_MapRGB(screen->format, 0x00, 0x37, 0xFF);
    text_color_bg_pressed = SDL_MapRGB(screen->format, 0x03, 0xF8, 0xFC);

    // setup keys
    keys = keys_lower;

    // set vars to initial value
    mouse_pressed = 0;
    mouse_just_pressed = 0;
}

void keyboard_setup(char* input_ptr, void (*enter_func_ptr)()) {
    // set input to use the passed in input string
    input = input_ptr;
    // set enter_func to the function passed in
    enter_func = enter_func_ptr;
}

void keyboard_clear_enter_func() {
    enter_func = NULL;
}

void show_keyboard(int input_x, int input_y) {
    // print input at specified location
    rect.x = input_x;
    rect.y = input_y;
    // create text with the font and color
    SDL_Surface *text = TTF_RenderText_Solid(font, input, text_color_fg);
    // blit the text to the screen
    SDL_BlitSurface(text, NULL, screen, &rect);
    // free the text to prevent memory leaks
    SDL_FreeSurface(text);

    // print each key
    int i, j;
    for(i = 0; i < KEYS_ROWS; i++) {
        rect.y = KEYBOARD_START_Y + (i * KEYBOARD_Y_INCREMENT) + KEYBOARD_KEY_SPACING;
        int len = strlen(keys[i]);

        // back quote and tilde keys special case
        if(i == 0) {
            rect.x = KEYBOARD_START_X - KEYBOARD_KEY_SPACING - KEYBOARD_KEY_BACKGROUND_SPACE;
            if(key_state == KEYS_LOWER)
                special_key_button("`", SPECIAL_KEY_ACTION_BACKQUOTE);
            else if(key_state == KEYS_UPPER || key_state == KEYS_EXTRA)
                special_key_button("~", SPECIAL_KEY_ACTION_TILDE);
        }

        if(i == 2) {
            rect.x = KEYBOARD_START_X - KEYBOARD_KEY_SPACING - KEYBOARD_KEY_BACKGROUND_SPACE;
            special_key_button("[c]", SPECIAL_KEY_ACTION_CAPS);
        }

        if(i == 3) {
            rect.x = KEYBOARD_START_X - KEYBOARD_KEY_BACKGROUND_SPACE;
            special_key_button("[s]", SPECIAL_KEY_ACTION_SHIFT);
        }

        for(j = 0; j < len; j++) {
            rect.x = KEYBOARD_START_X + (j * KEYBOARD_X_INCREMENT) + KEYBOARD_KEY_SPACING + i * KEYBOARD_KEY_SPACING;
            key_button(keys[i][j]);
        }

        if(i == 0) {
            rect.x += KEYBOARD_KEY_SPACING * 2 + KEYBOARD_KEY_BACKGROUND_SPACE;
            special_key_button("<==", SPECIAL_KEY_ACTION_BACKSPACE);
        }

        if(i == 2) {
            rect.x += KEYBOARD_KEY_SPACING * 2 + KEYBOARD_KEY_BACKGROUND_SPACE;
            special_key_button("==>", SPECIAL_KEY_ACTION_ENTER);
        }

        if(i == 3) {
            rect.x += KEYBOARD_KEY_SPACING * 2 + KEYBOARD_KEY_BACKGROUND_SPACE;
            special_key_button("[e]", SPECIAL_KEY_ACTION_EXTRA);
        }
    }

    rect.y = KEYBOARD_START_Y + (4 * KEYBOARD_Y_INCREMENT) + KEYBOARD_KEY_SPACING;
    rect.x = KEYBOARD_START_X + KEYBOARD_KEY_SPACING * 5;
    special_key_button("        ", SPECIAL_KEY_ACTION_SPACE);

    rect.x = KEYBOARD_START_X + KEYBOARD_KEY_SPACING * 19 + KEYBOARD_KEY_BACKGROUND_SPACE;
    special_key_button("cls", SPECIAL_KEY_ACTION_CLEAR);
}

void draw_bottom_screen_bounds() {
    #define TOP_SCREEN_WIDTH 400
    #define TOP_SCREEN_HEIGHT 240
    #define BOTTOM_SCREEN_OFFSET 40
    #define BOTTOM_SCREEN_HEIGHT 240

    SDL_Rect bounds;
    bounds.x = 0;
    bounds.y = TOP_SCREEN_HEIGHT;
    bounds.w = BOTTOM_SCREEN_OFFSET;
    bounds.h = BOTTOM_SCREEN_HEIGHT;
    SDL_FillRect(screen, &bounds, 0xFFFFFFFF);

    bounds.x = TOP_SCREEN_WIDTH - BOTTOM_SCREEN_OFFSET;
    bounds.y = TOP_SCREEN_HEIGHT;
    bounds.w = BOTTOM_SCREEN_OFFSET;
    bounds.h = BOTTOM_SCREEN_HEIGHT;
    SDL_FillRect(screen, &bounds, 0xFFFFFFFF);
}

void debug_dual_screen() {
    #define TOP_SCREEN_WIDTH 400
    #define TOP_SCREEN_HEIGHT 240
    #define BOX_SIZE 10

    SDL_Rect bounds;
    bounds.x = 0;
    bounds.y = 0;
    bounds.w = BOX_SIZE;
    bounds.h = BOX_SIZE;

    SDL_FillRect(screen, &bounds, 0xFFFFFFFF);

    bounds.x = TOP_SCREEN_WIDTH - BOX_SIZE;
    bounds.y = TOP_SCREEN_HEIGHT - BOX_SIZE;
    SDL_FillRect(screen, &bounds, 0xFFFFFFFF);

    #define BOTTOM_SCREEN_OFFSET 40
    #define BOTTOM_SCREEN_WIDTH 320
    #define BOTTOM_SCREEN_HEIGHT 240

    bounds.x = BOTTOM_SCREEN_OFFSET;
    bounds.y = TOP_SCREEN_HEIGHT;
    SDL_FillRect(screen, &bounds, 0xFFFFFFFF);

    bounds.x = BOTTOM_SCREEN_OFFSET + BOTTOM_SCREEN_WIDTH - BOX_SIZE;
    bounds.y = TOP_SCREEN_HEIGHT + BOTTOM_SCREEN_HEIGHT - BOX_SIZE;
    SDL_FillRect(screen, &bounds, 0xFFFFFFFF);
}

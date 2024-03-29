#ifndef INTERFACE_KEYBOARD_H_INCLUDED
#define INTERFACE_KEYBOARD_H_INCLUDED

#define KEYS_ROWS 4
#define KEYBOARD_START_X 62
#define KEYBOARD_START_Y 280
#define KEYBOARD_Y_INCREMENT 30
#define KEYBOARD_X_INCREMENT 25
#define KEYBOARD_KEY_SPACING 10
#define KEYBOARD_KEY_BACKGROUND_SPACE 5
#define MAX_INPUT_LENGTH 100

#define SPECIAL_KEY_ACTION_BACKSPACE 1
#define SPECIAL_KEY_ACTION_ENTER 2
#define SPECIAL_KEY_ACTION_CAPS 3
#define SPECIAL_KEY_ACTION_SHIFT 4
#define SPECIAL_KEY_ACTION_SPACE 5
#define SPECIAL_KEY_ACTION_TILDE 6
#define SPECIAL_KEY_ACTION_BACKQUOTE 7
#define SPECIAL_KEY_ACTION_EXTRA 8
#define SPECIAL_KEY_ACTION_CLEAR 9

#define KEYS_LOWER 0
#define KEYS_UPPER 1
#define KEYS_EXTRA 2

void keyboard_init();
void show_keyboard(int input_x, int input_y);
void keyboard_setup(char* input_ptr, void (*enter_func_ptr)());
void keyboard_clear_enter_func();

#endif

#ifndef UTILS_H
#define UTILS_H

#include "common.h"

void utils_clear_buffer(void);
void utils_wait_enter(void);
int utils_input_int(const char *prompt, int min, int max);
float utils_input_float(const char *prompt, float min, float max);
void utils_input_string(const char *prompt, char *buffer, int size);
int utils_confirm(const char *prompt);
void ui_print_main_title(void);
void ui_print_sub_title(const char *title);
void ui_print_line(void);

#endif

#include <stdio.h>
#include "utils.h"

void print_message(const char *msg) {
    if (msg != NULL) {
        printf("%s\n", msg);
    }
}

int validate_input(int value) {
    return (value >= 0) ? SUCCESS : FAILURE;
}

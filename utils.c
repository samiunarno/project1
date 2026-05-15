#include "utils.h"

void utils_clear_buffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

void utils_wait_enter(void) {
    printf("\n按回车继续...");
    fflush(stdout);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

static int parse_int_line(const char *line, int *out) {
    char extra;
    return sscanf(line, "%d %c", out, &extra) == 1;
}

static int parse_float_line(const char *line, float *out) {
    char extra;
    return sscanf(line, "%f %c", out, &extra) == 1;
}

int utils_input_int(const char *prompt, int min, int max) {
    char line[128];
    int value;
    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            clearerr(stdin);
            continue;
        }
        if (!parse_int_line(line, &value)) {
            printf("输入有误，请重新输入！\n");
            continue;
        }
        if (value < min || value > max) {
            printf("输入超出范围，请重新输入！\n");
            continue;
        }
        return value;
    }
}

float utils_input_float(const char *prompt, float min, float max) {
    char line[128];
    float value;
    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            clearerr(stdin);
            continue;
        }
        if (!parse_float_line(line, &value)) {
            printf("输入有误，请重新输入！\n");
            continue;
        }
        if (value < min || value > max) {
            printf("输入超出范围，请重新输入！\n");
            continue;
        }
        return value;
    }
}

void utils_input_string(const char *prompt, char *buffer, int size) {
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, size, stdin)) {
            clearerr(stdin);
            continue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) {
            printf("输入不能为空，请重新输入！\n");
            continue;
        }
        return;
    }
}

int utils_confirm(const char *prompt) {
    char line[16];
    while (1) {
        printf("%s (y/n)：", prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            clearerr(stdin);
            continue;
        }
        if (line[0] == 'y' || line[0] == 'Y') return 1;
        if (line[0] == 'n' || line[0] == 'N') return 0;
        printf("请输入 y 或 n。\n");
    }
}

void ui_print_main_title(void) {
    printf("==================================================\n");
    printf("              轻量级医院信息管理系统\n");
    printf("==================================================\n");
}

void ui_print_sub_title(const char *title) {
    printf("\n---------------- %s ----------------\n", title);
}

void ui_print_line(void) {
    printf("--------------------------------------------------\n");
}

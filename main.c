#include <stdio.h>
#include "common.h"

/* Module D functions */
extern void init_hospital_globals(void);
extern int A_entry(void);
extern int B_entry(void);
extern int C_entry(void);
extern int D_entry(void);

int main(void) {
    int choice;
    int status = SUCCESS;

    printf("============================================================\n");
    printf("        医院管理系统正在启动\n");
    printf("============================================================\n");

    /* Initialize Hospital System */
    init_hospital_globals();

    while(1) {
        printf("\n-------------- 主菜单 --------------\n");
        printf("1. 进入模块 A\n");
        printf("2. 进入模块 B\n");
        printf("3. 进入模块 C\n");
        printf("4. 进入模块 D\n");
        printf("0. 退出系统\n");
        printf("---------------------------------------\n");
        printf("请输入您的选择: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                printf("\n--- 正在进入模块 A ---\n");
                status = A_entry();
                break;
            case 2:
                printf("\n--- 正在进入模块 B ---\n");
                status = B_entry();
                break;
            case 3:
                printf("\n--- 正在进入模块 C ---\n");
                status = C_entry();
                break;
            case 4:
                printf("\n--- 正在进入模块 D ---\n");
                status = D_entry();
                break;
            case 0:
                printf("\n正在退出系统...\n");
                goto exit_program;
            default:
                printf("\n输入无效！请重试。\n");
                break;
        }

        if(status != SUCCESS) {
            printf("\n模块执行时发生错误。\n");
        }
    }

exit_program:
    printf("\n============================================================\n");
    printf("        医院管理系统已关闭\n");
    printf("============================================================\n");

    return SUCCESS;
}
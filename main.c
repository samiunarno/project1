#include <stdio.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif

int A_entry(void);
int B_entry(void);
int C_entry(void);
int D_entry(void);

static void foundation_setup_console(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, ".UTF-8");
#else
    setlocale(LC_ALL, "");
#endif
}

static void clear_input_buffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

static void print_main_header(void) {
    printf("\n============================================================\n");
    printf("                     轻量级医疗管理系统\n");
    printf("============================================================\n");
    printf("请选择需要进入的业务功能（各模块内支持 0 返回上一步）：\n");
    printf("  1. 门诊患者与挂号服务\n");
    printf("     - 新增患者 / 查询患者 / 修改患者 / 删除患者 / 挂号记录\n");
    printf("  2. 医生接诊与候诊队列\n");
    printf("     - 科室医生查询 / 候诊队列 / 叫号 / 完成接诊\n");
    printf("  3. 药房与处方服务\n");
    printf("     - 药品维护 / 库存操作 / 创建处方 / 处方查询\n");
    printf("  4. 住院与统计服务\n");
    printf("     - 入院 / 转床转科 / 出院 / 床位管理 / 统计报表\n");
    printf("  0. 退出系统\n");
    printf("------------------------------------------------------------\n");
}

static void print_enter_tip(int choice) {
    switch (choice) {
        case 1:
            printf("\n>>> 进入【门诊患者与挂号服务】...\n");
            break;
        case 2:
            printf("\n>>> 进入【医生接诊与候诊队列】...\n");
            break;
        case 3:
            printf("\n>>> 进入【药房与处方服务】...\n");
            break;
        case 4:
            printf("\n>>> 进入【住院与统计服务】...\n");
            break;
        default:
            break;
    }
}

int main(void) {
    foundation_setup_console();

    while (1) {
        int choice;
        print_main_header();
        printf("请输入序号：");

        if (scanf("%d", &choice) != 1) {
            printf("输入有误，请输入数字序号。\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1:
                print_enter_tip(choice);
                A_entry();
                break;
            case 2:
                print_enter_tip(choice);
                B_entry();
                break;
            case 3:
                print_enter_tip(choice);
                C_entry();
                break;
            case 4:
                print_enter_tip(choice);
                D_entry();
                break;
            case 0:
                printf("\n感谢使用轻量级医疗管理系统，再见。\n");
                return 0;
            default:
                printf("无效选项，请重新输入。\n");
                break;
        }
    }
}

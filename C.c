#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * C_part_combined.c
 * 合并来源：
 * 1. prescription.h
 * 2. prescription.c
 * 3. pharmacy(1).h
 * 4. pharmacy(1).c
 *
 * 说明：
 * - 尽量保留了原有函数名、变量名、整体写法和逻辑。
 * - 为避免重复定义，Medicine 结构体只保留一份。
 * - prescription.c 里原本的 static medicine_select 与 pharmacy(1).c 中
 *   的 medicine_select 功能重复，这里统一保留一个版本供两边共用。
 * - pharmacy(1).c 里的 main 保留为条件编译形式，避免后续并入总工程时
 *   和项目主入口冲突；若要单独演示本文件，可定义 C_MODULE_STANDALONE。
 */

typedef struct Registration {
    int registerId;
    int patientId;
    int doctorId;
    int status;
    struct Registration* next;
} Registration;

Registration* c_regHead = NULL;

typedef struct Medicine {
    char* name;
    int price;
    int stock;
    int warningLine;
    struct Medicine* next;
} Medicine;

/* 处方明细 */
typedef struct PrescriptionItem {
    char medicineName[50];
    int quantity;
    int itemCost;
    struct PrescriptionItem* next;
} PrescriptionItem;

/* 处方主体 */
typedef struct Prescription {
    int prescriptionId;
    int patientId;
    int totalCost;
    PrescriptionItem* items;
    struct Prescription* next;
} Prescription;

/* 全局处方链表 */
Prescription* presHead = NULL;
int curPresId = 4001;


/* ===== 来自 prescription.h 的函数声明 ===== */
int prescription_check_visit_status(int patientId);
int prescription_create_for_called_patient(int patientId);
int prescription_add_item_by_name(Medicine* medHead, int presId, char* medName, int num);
void prescription_display_one(int presId);
void prescription_display_all(void);
void prescription_display_by_patient(int patientId);
void prescription_free_all(void);

/* ===== 来自 pharmacy(1).h 的函数声明 ===== */
void clearInputBuffer(void);
Medicine* medicine_in(Medicine** head, char* name, int price, int stock, int warningLine);
Medicine* medicine_select(Medicine* head, char* name);
void medicine_delete(Medicine** head, char* name);
void medicine_menu(Medicine** head);
void medicine_stock(Medicine* head);
void medicine_print(Medicine* head);
void saveToFile(Medicine *head);
void loadFromFile(Medicine **head);

/* ====================== prescription.c ====================== */

/* 修正：校验患者是否处于"就诊中"状态（status=2） */
int prescription_check_visit_status(int patientId)
{
    /* 1. 基础ID合法性校验 */
    if(patientId < 1001)
    {
        printf("--------------------------------------------------\n");
        printf("患者编号非法，禁止开处方！\n");
        return 0;
    }

    /* 2. 遍历挂号链表，校验患者是否处于就诊中（status=2） */
    Registration* reg = c_regHead;
    int isVisiting = 0;
    while(reg != NULL)
    {
        if(reg->patientId == patientId && reg->status == 2)
        {
            isVisiting = 1;
            break;
        }
        reg = reg->next;
    }

    /* 3. 状态判断 */
    if(!isVisiting)
    {
        printf("--------------------------------------------------\n");
        printf("患者未挂号/未处于就诊中状态，禁止开处方！\n");
        return 0;
    }

    printf("--------------------------------------------------\n");
    printf("患者就诊状态校验通过，允许开处方\n");
    return 1;
}

/* 以下函数保持原有逻辑不变 */
int prescription_create_for_called_patient(int patientId)
{
    if(!prescription_check_visit_status(patientId))
        return 0;

    Prescription* newPres = (Prescription*)malloc(sizeof(Prescription));
    if(!newPres)
    {
        printf("内存分配失败！\n");
        return 0;
    }
    newPres->prescriptionId = curPresId++;
    newPres->patientId = patientId;
    newPres->totalCost = 0;
    newPres->items = NULL;
    newPres->next = NULL;

    /* 尾插 */
    if(presHead == NULL)
        presHead = newPres;
    else
    {
        Prescription* p = presHead;
        while(p->next) p = p->next;
        p->next = newPres;
    }
    printf("操作成功！处方创建成功，处方编号：%d\n", newPres->prescriptionId);
    return 1;
}

int prescription_add_item_by_name(Medicine* medHead, int presId, char* medName, int num)
{
    if(num <= 0)
    {
        printf("输入有误，请重新输入！药品数量不能小于等于0\n");
        return 0;
    }

    /* 找处方 */
    Prescription* p = presHead;
    while(p && p->prescriptionId != presId)
        p = p->next;
    if(!p)
    {
        printf("未找到对应处方！\n");
        return 0;
    }

    /* 找药品（复用你药房查找） */
    Medicine* med = medicine_select(medHead, medName);
    if(!med) return 0;

    /* 库存判断（对齐你出库逻辑） */
    if(med->stock < num)
    {
        printf("库存不足！\n");
        return 0;
    }

    /* 新建明细 */
    PrescriptionItem* item = (PrescriptionItem*)malloc(sizeof(PrescriptionItem));
    if(!item)
    {
        printf("内存分配失败！\n");
        return 0;
    }
    strcpy(item->medicineName, medName);
    item->quantity = num;
    item->itemCost = med->price * num;
    item->next = NULL;

    /* 明细尾插 */
    if(!p->items)
        p->items = item;
    else
    {
        PrescriptionItem* q = p->items;
        while(q->next) q = q->next;
        q->next = item;
    }

    /* 累加总费用 */
    p->totalCost += item->itemCost;
    /* 扣库存 和你药房出库逻辑完全一致 */
    med->stock -= num;

    printf("操作成功！药品已加入处方并扣减库存\n");
    return 1;
}

void prescription_display_one(int presId)
{
    Prescription* p = presHead;
    while(p && p->prescriptionId != presId)
        p = p->next;
    if(!p)
    {
        printf("未找到对应信息！\n");
        return;
    }

    printf("---------------- 处方信息 ----------------\n");
    printf("处方编号：%d\n", p->prescriptionId);
    printf("患者编号：%d\n", p->patientId);
    printf("处方总费用：%d 元\n", p->totalCost);
    printf("----------------------------------------\n");
    printf("药品名称\t数量\t单品费用\n");

    PrescriptionItem* item = p->items;
    if(!item)
    {
        printf("暂无药品明细\n");
        return;
    }
    while(item)
    {
        printf("%s\t%d\t%d\n",
               item->medicineName,
               item->quantity,
               item->itemCost);
        item = item->next;
    }
    printf("--------------------------------------------------\n");
}

void prescription_display_all(void)
{
    if(!presHead)
    {
        printf("暂无处方记录！\n");
        return;
    }
    Prescription* p = presHead;
    while(p)
    {
        prescription_display_one(p->prescriptionId);
        p = p->next;
    }
}

void prescription_display_by_patient(int patientId)
{
    int flag = 0;
    Prescription* p = presHead;
    while(p)
    {
        if(p->patientId == patientId)
        {
            prescription_display_one(p->prescriptionId);
            flag = 1;
        }
        p = p->next;
    }
    if(!flag)
        printf("该患者暂无处方记录！\n");
}

void prescription_free_all(void)
{
    Prescription* p = presHead;
    while(p)
    {
        Prescription* t1 = p;
        p = p->next;

        PrescriptionItem* item = t1->items;
        while(item)
        {
            PrescriptionItem* t2 = item;
            item = item->next;
            free(t2);
        }
        free(t1);
    }
    presHead = NULL;
}

/* ====================== pharmacy(1).c ====================== */

void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

Medicine* medicine_in(Medicine** head, char* name, int price, int stock, int warningLine) {
    Medicine* medicine_new = (Medicine*)malloc(sizeof(Medicine));
    if (medicine_new == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    medicine_new->name = (char*)malloc(strlen(name) + 1);
    if (medicine_new->name == NULL) {
        printf("药品名称内存分配失败！\n");
        free(medicine_new);
        return NULL;
    }
    strcpy(medicine_new->name, name);
    medicine_new->price = price;
    medicine_new->stock = stock;
    medicine_new->warningLine = warningLine;
    medicine_new->next = NULL;

    if (*head == NULL) {
        *head = medicine_new;
        return medicine_new;
    }
    Medicine* last = *head;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = medicine_new;
    return medicine_new;
}

Medicine* medicine_select(Medicine* head, char* name) {
    Medicine* last = head;
    while (last != NULL) {
        if (strcmp(last->name, name) == 0) {
            return last;
        }
        last = last->next;
    }
    printf("未找到药品！\n");
    return NULL;
}

void medicine_delete(Medicine** head, char* name) {
    if (*head == NULL) {
        printf("药品列表为空，删除失败！\n");
        return;
    }
    Medicine* last = *head;
    Medicine* pre = NULL;
    while (last != NULL) {
        if (strcmp(last->name, name) == 0) {
            free(last->name);
            if (pre == NULL) {
                *head = last->next;
            } else {
                pre->next = last->next;
            }
            free(last);
            printf("成功删除药品：%s\n", name);
            return;
        }
        pre = last;
        last = last->next;
    }
    printf("未找到药品：%s\n", name);
}

/* ====================== 修复：参数改为二级指针 ====================== */
void medicine_stock(Medicine* head) {
    int num;
    char name[50];
    int add;
    int out;
    printf("---------------- 库存操作 ----------------\n1.入库\n2.出库\n0.返回上一级菜单\n请输入选项：");
    scanf("%d", &num);
    clearInputBuffer();

    if (num == 1) {
        printf("请输入药品名称：");
        scanf("%s", name);
        if (strcmp(name, "0") == 0) { clearInputBuffer(); return; }
        Medicine* p = medicine_select(head, name);
        if (p == NULL) {
            printf("未找到药品！\n");
            return;
        }
        printf("请输入入库数量：");
        a:
        while (scanf("%d", &add) != 1) {
            clearInputBuffer();
            printf("输入无效，请重新输入：");
        }
        if (add == 0) { return; }
        if (add < 0) {
            printf("入库量不得小于0！请重新输入:");
            goto a;
        }
        p->stock += add;
        printf("入库成功！\n");
    } else if (num == 2) {
        printf("请输入药品名称：");
        scanf("%s", name);
        if (strcmp(name, "0") == 0) { clearInputBuffer(); return; }
        Medicine* p = medicine_select(head, name);
        if (p == NULL) {
            printf("未找到药品！\n");
            return;
        }
        printf("请输入出库数量：");
        b:
        while (scanf("%d", &out) != 1) {
            clearInputBuffer();
            printf("输入无效，请重新输入：");
        }
        if (out == 0) { return; }
        if (p->stock < out) {
            printf("库存不可小于0！请重新输入：");
            goto b;
        }
        p->stock -= out;
        printf("出库成功！\n");
    } else if (num == 0) {
        /* 修复：不再递归，直接返回 */
        return;
    } else {
        printf("未查询到有效操作！\n");
    }
}

/* ====================== 修复：二级指针 ====================== */
void medicine_menu(Medicine** head) {
    int num;
    char name1[50] = {0};
    char name2[50] = {0};
    int price;
    int stock;
    int warningLine;

    printf("---------------- 药品操作 ----------------\n1.添加药品\n2.删除药品\n3.库存操作\n0.返回\n请输入选项：");
    scanf("%d", &num);
    clearInputBuffer();

    if (num == 1) {
        printf("药品名称：");
        scanf("%s", name1);
        if (strcmp(name1, "0") == 0) { clearInputBuffer(); return; }
        printf("药品价格：");
        scanf("%d", &price);
        printf("药品库存：");
        scanf("%d", &stock);
        printf("库存预警线：");
        scanf("%d", &warningLine);
        medicine_in(head, name1, price, stock, warningLine);
        printf("添加成功！\n");
    } else if (num == 2) {
        printf("请输入药品名称：");
        scanf("%s", name2);
        if (strcmp(name2, "0") == 0) { clearInputBuffer(); return; }
        medicine_delete(head, name2);
    } else if (num == 3) {
        medicine_stock(*head);
    } else if (num != 0) {
        printf("无效选项\n");
    }
}

void medicine_print(Medicine* head) {
    Medicine* last = head;
    int i = 1;
    while (last != NULL) {
        printf("===== 药品 %d =====\n名称：%s\n价格：%d\n库存：%d\n预警线：%d\n",
               i, last->name, last->price, last->stock, last->warningLine);
        i++;
        last = last->next;
    }
}

/* ====================== 文件联动 ====================== */
void saveToFile(Medicine *head) {
    FILE *fp = fopen("medicine.txt", "w");
    if (fp == NULL) {
        printf("文件打开失败，无法保存\n");
        return;
    }
    Medicine *p = head;
    while (p != NULL) {
        fprintf(fp, "%s %d %d %d\n", p->name, p->price, p->stock, p->warningLine);
        p = p->next;
    }
    fclose(fp);
    printf("已保存所有药品到文件 medicine.txt\n");
}

void loadFromFile(Medicine **head) {
    FILE *fp = fopen("medicine.txt", "r");
    if (fp == NULL) {
        printf("暂无历史数据，新建空链表\n");
        return;
    }
    char name[100];
    int price, stock, warningLine;
    while (fscanf(fp, "%s %d %d %d", name, &price, &stock, &warningLine) != EOF) {
        medicine_in(head, name, price, stock, warningLine);
    }
    fclose(fp);
    printf("已从文件加载药品数据\n");
}

/* ====================== 原 pharmacy(1).c 主函数（条件保留） ====================== */
#ifdef C_MODULE_STANDALONE
int main() {
    Medicine *head = NULL;

    loadFromFile(&head);

    int op;
    while (1) {
        printf("\n===== 主菜单 =====\n");
        printf("1.药品操作\n2.打印药品\n3.保存并退出\n请选择：");
        scanf("%d", &op);
        clearInputBuffer();

        if (op == 1) {
            medicine_menu(&head);
        } else if (op == 2) {
            medicine_print(head);
        } else if (op == 3) {
            saveToFile(head);
            break;
        } else {
            printf("无效选项\n");
        }
    }

    return 0;
}
#endif


static void c_prepare_demo_registration(void) {
    if (c_regHead != NULL) return;
    Registration* r = (Registration*)malloc(sizeof(Registration));
    if (!r) return;
    r->registerId = 7001;
    r->patientId = 1001;
    r->doctorId = 2001;
    r->status = 2;
    r->next = NULL;
    c_regHead = r;
}

int C_entry(void) {
    Medicine *head = NULL;
    int op;
    c_prepare_demo_registration();

    while (1) {
        printf("\n===== C 模块菜单 =====\n");
        printf("1. 药品操作\n");
        printf("2. 打印药品\n");
        printf("3. 创建示例处方(患者1001)\n");
        printf("4. 给处方添加药品\n");
        printf("5. 显示全部处方\n");
        printf("0. 返回上一级\n请选择：");
        if (scanf("%d", &op) != 1) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (op == 1) {
            medicine_menu(&head);
        } else if (op == 2) {
            medicine_print(head);
        } else if (op == 3) {
            prescription_create_for_called_patient(1001);
        } else if (op == 4) {
            int presId = 0, num = 0, step = 0;
            char medName[100];
            while (1) {
                if (step == 0) {
                    printf("请输入处方编号：");
                    if (scanf("%d", &presId) != 1) { clearInputBuffer(); continue; }
                    clearInputBuffer();
                    if (presId == 0) break;
                    step = 1;
                } else if (step == 1) {
                    printf("请输入药品名称：");
                    if (scanf("%99s", medName) != 1) { clearInputBuffer(); continue; }
                    clearInputBuffer();
                    if (strcmp(medName, "0") == 0) { step = 0; continue; }
                    step = 2;
                } else {
                    printf("请输入药品数量：");
                    if (scanf("%d", &num) != 1) { clearInputBuffer(); continue; }
                    clearInputBuffer();
                    if (num == 0) { step = 1; continue; }
                    prescription_add_item_by_name(head, presId, medName, num);
                    break;
                }
            }
        } else if (op == 5) {
            prescription_display_all();
        } else if (op == 0) {
            saveToFile(head);
            prescription_free_all();
            while (head) {
                Medicine *tmp = head;
                head = head->next;
                free(tmp->name);
                free(tmp);
            }
            while (c_regHead) {
                Registration *tmp = c_regHead;
                c_regHead = c_regHead->next;
                free(tmp);
            }
            return 0;
        } else {
            printf("无效选项\n");
        }
    }
}

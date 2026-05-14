#include "common.h"
#include "utils.h"
#include <ctype.h>

/* 门诊管理模块：科室 / 医生 / 住院 / 病历 / 检查 / 处方 / 队列 */

static Patient *patient_head = NULL;
static Doctor *doctor_head = NULL;
static Medicine *medicine_head = NULL;
static Prescription *prescription_head = NULL;
static Registration *reg_head = NULL;
static QueueNode *queue_head = NULL;

static int next_record_id = 1001;
static int next_check_id = 5001;
static int next_prescription_id = 8001;
static int next_reg_id = 10001;
static int next_queue_id = 1000;
static int module_inited = 0;

static char dept_list[100][DEPT_LEN];
static int dept_count = 0;

/* ===================== 工具函数 ===================== */

static int input_int_with_back(const char *prompt, int min, int max, int *back) {
    if (back) *back = 0;
    while (1) {
        int value = utils_input_int(prompt, 0, max);
        if (value == 0) {
            if (back) *back = 1;
            return 0;
        }
        if (value < min) {
            printf("输入超出范围，请重新输入！\n");
            continue;
        }
        return value;
    }
}

static int input_string_with_back(const char *prompt, char *dest, int maxLen) {
    utils_input_string(prompt, dest, maxLen);
    return strcmp(dest, "0") != 0;
}

/* ===================== 查找函数 ===================== */

static Doctor *query_doctor_by_id(int doctor_id) {
    Doctor *p = doctor_head;
    while (p) {
        if (p->doctorId == doctor_id) return p;
        p = p->next;
    }
    return NULL;
}

static Patient *query_patient_by_id(int pid) {
    Patient *p = patient_head;
    while (p) {
        if (p->patientId == pid) return p;
        p = p->next;
    }
    return NULL;
}

static Medicine *query_medicine_by_id(int mid) {
    Medicine *m = medicine_head;
    while (m) {
        if (m->medicineId == mid) return m;
        m = m->next;
    }
    return NULL;
}

/* ===================== 内存释放 ===================== */

static void free_records(VisitRecord *head) {
    while (head) {
        VisitRecord *tmp = head;
        head = head->next;
        free(tmp);
    }
}

static void free_checks(CheckRecord *head) {
    while (head) {
        CheckRecord *tmp = head;
        head = head->next;
        free(tmp);
    }
}

static void free_prescription_items(PrescriptionItem *head) {
    while (head) {
        PrescriptionItem *tmp = head;
        head = head->next;
        free(tmp);
    }
}

static void free_prescriptions(Prescription *head) {
    while (head) {
        free_prescription_items(head->items);
        Prescription *tmp = head;
        head = head->next;
        free(tmp);
    }
}

static void free_patients(void) {
    Patient *p = patient_head;
    while (p) {
        free_records(p->records);
        free_checks(p->checks);
        Patient *tmp = p;
        p = p->next;
        free(tmp);
    }
    patient_head = NULL;
}

static void free_doctors(void) {
    Doctor *d = doctor_head;
    while (d) {
        Doctor *tmp = d;
        d = d->next;
        free(tmp);
    }
    doctor_head = NULL;
}

static void free_medicines(void) {
    Medicine *m = medicine_head;
    while (m) {
        Medicine *tmp = m;
        m = m->next;
        free(tmp);
    }
    medicine_head = NULL;
}

static void free_regs(void) {
    Registration *r = reg_head;
    while (r) {
        Registration *tmp = r;
        r = r->next;
        free(tmp);
    }
    reg_head = NULL;
}

static void free_queue(void) {
    QueueNode *q = queue_head;
    while (q) {
        QueueNode *tmp = q;
        q = q->next;
        free(tmp);
    }
    queue_head = NULL;
}

static void free_all_memory(void) {
    free_patients();
    free_doctors();
    free_medicines();
    free_prescriptions(prescription_head);
    prescription_head = NULL;
    free_regs();
    free_queue();
}

/* ===================== 初始化 ===================== */

static void init_base_data(void) {
    Doctor *d1, *d2, *d3, *d4, *d5;
    Medicine *m1, *m2, *m3, *m4;

    free_doctors();
    free_medicines();

    d1 = (Doctor *)malloc(sizeof(Doctor));
    d2 = (Doctor *)malloc(sizeof(Doctor));
    d3 = (Doctor *)malloc(sizeof(Doctor));
    d4 = (Doctor *)malloc(sizeof(Doctor));
    d5 = (Doctor *)malloc(sizeof(Doctor));
    if (!d1 || !d2 || !d3 || !d4 || !d5) {
        printf("内存分配失败！\n");
        free(d1);
        free(d2);
        free(d3);
        free(d4);
        free(d5);
        doctor_head = NULL;
        return;
    }

    d1->doctorId = 101;
    strcpy(d1->name, "张医生");
    strcpy(d1->department, "内科");
    strcpy(d1->title, "主任医师");
    d1->patientCount = 0;
    d1->next = NULL;

    d2->doctorId = 102;
    strcpy(d2->name, "李医生");
    strcpy(d2->department, "外科");
    strcpy(d2->title, "副主任医师");
    d2->patientCount = 0;
    d2->next = d1;

    d3->doctorId = 103;
    strcpy(d3->name, "王医生");
    strcpy(d3->department, "内科");
    strcpy(d3->title, "主治医师");
    d3->patientCount = 0;
    d3->next = d2;

    d4->doctorId = 104;
    strcpy(d4->name, "赵医生");
    strcpy(d4->department, "儿科");
    strcpy(d4->title, "主治医师");
    d4->patientCount = 0;
    d4->next = d3;

    d5->doctorId = 105;
    strcpy(d5->name, "刘医生");
    strcpy(d5->department, "骨科");
    strcpy(d5->title, "主任医师");
    d5->patientCount = 0;
    d5->next = d4;
    doctor_head = d5;

    m1 = (Medicine *)malloc(sizeof(Medicine));
    m2 = (Medicine *)malloc(sizeof(Medicine));
    m3 = (Medicine *)malloc(sizeof(Medicine));
    m4 = (Medicine *)malloc(sizeof(Medicine));
    if (!m1 || !m2 || !m3 || !m4) {
        printf("内存分配失败！\n");
        free(m1);
        free(m2);
        free(m3);
        free(m4);
        return;
    }

    m1->medicineId = 2001;
    strcpy(m1->name, "感冒灵颗粒");
    m1->price = 18.5;
    m1->stock = 100;
    m1->warningLine = 20;
    m1->next = NULL;

    m2->medicineId = 2002;
    strcpy(m2->name, "布洛芬缓释胶囊");
    m2->price = 25.0;
    m2->stock = 80;
    m2->warningLine = 15;
    m2->next = m1;

    m3->medicineId = 2003;
    strcpy(m3->name, "阿莫西林");
    m3->price = 32.8;
    m3->stock = 60;
    m3->warningLine = 10;
    m3->next = m2;

    m4->medicineId = 2004;
    strcpy(m4->name, "维生素C片");
    m4->price = 12.0;
    m4->stock = 200;
    m4->warningLine = 30;
    m4->next = m3;
    medicine_head = m4;

    module_inited = 1;
}

/* ===================== 科室与医生查询 ===================== */

static void query_all_department(void) {
    Doctor *p;
    int i;
    int exist;

    if (!doctor_head) {
        printf("暂无医生数据，无法查询科室！\n");
        dept_count = 0;
        return;
    }
    dept_count = 0;
    p = doctor_head;
    printf("\n=== 医院所有科室 ===\n");
    while (p) {
        exist = 0;
        for (i = 0; i < dept_count; i++) {
            if (strcmp(dept_list[i], p->department) == 0) {
                exist = 1;
                break;
            }
        }
        if (!exist) {
            strcpy(dept_list[dept_count++], p->department);
            printf("  %d. %s\n", dept_count, p->department);
        }
        p = p->next;
    }
    if (dept_count == 0) {
        printf("暂无科室信息！\n");
    }
}

static void query_doctor_by_dept(const char *dept_name) {
    Doctor *p;
    int count;

    if (!doctor_head) {
        printf("暂无医生数据！\n");
        return;
    }
    p = doctor_head;
    count = 0;
    printf("\n=== %s 科室医生列表 ===\n", dept_name);
    printf("%-8s %-10s %-12s %-10s\n", "医生ID", "姓名", "职称", "接诊人数");
    printf("-------------------------------------------\n");
    while (p) {
        if (strcmp(p->department, dept_name) == 0) {
            count++;
            printf("%-8d %-10s %-12s %-10d\n",
                   p->doctorId, p->name, p->title, p->patientCount);
        }
        p = p->next;
    }
    if (count == 0) printf("该科室暂无医生！\n");
}

static void query_doctor_workload(void) {
    Doctor **docs;
    Doctor *p;
    int count;
    int i, j;

    if (!doctor_head) {
        printf("暂无医生数据！\n");
        return;
    }

    count = 0;
    p = doctor_head;
    while (p) {
        count++;
        p = p->next;
    }

    if (count == 0) {
        printf("暂无医生数据！\n");
        return;
    }

    docs = (Doctor **)malloc(count * sizeof(Doctor *));
    if (!docs) {
        printf("内存分配失败！\n");
        return;
    }

    p = doctor_head;
    for (i = 0; i < count; i++) {
        docs[i] = p;
        p = p->next;
    }

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (docs[j]->patientCount < docs[j + 1]->patientCount) {
                Doctor *tmp = docs[j];
                docs[j] = docs[j + 1];
                docs[j + 1] = tmp;
            }
        }
    }

    printf("\n===== 医生工作量统计（按接诊量排序）=====\n");
    printf("%-4s %-8s %-10s %-12s %-10s %-10s\n",
           "排名", "医生ID", "姓名", "科室", "职称", "接诊人数");
    printf("-------------------------------------------------------\n");
    for (i = 0; i < count; i++) {
        printf("%-4d %-8d %-10s %-12s %-10s %-10d\n",
               i + 1, docs[i]->doctorId, docs[i]->name, docs[i]->department,
               docs[i]->title, docs[i]->patientCount);
    }
    free(docs);
}

/* ===================== 住院状态管理 ===================== */

static void change_patient_hospital(void) {
    int pid;
    int opt;
    Patient *p;

    printf("\n===== 修改患者住院状态 =====\n");
    printf("请输入患者ID：");
    scanf("%d", &pid);
    while (getchar() != '\n' && getchar() != EOF);

    p = query_patient_by_id(pid);
    if (!p) {
        printf("【错误】查无此患者！\n解决方案：先录入患者建档后再操作\n");
        return;
    }

    printf("当前状态：%s\n", p->isHospitalized ? "住院" : "门诊");
    printf("1.标记住院   0.设置门诊不住院\n请选择：");
    scanf("%d", &opt);
    while (getchar() != '\n' && getchar() != EOF);
    if (opt == 1) {
        p->isHospitalized = 1;
        printf("已设置为住院患者\n");
    } else if (opt == 0) {
        p->isHospitalized = 0;
        printf("已设置为门诊患者\n");
    } else {
        printf("【错误】无效选项！\n");
    }
}

/* ===================== 病历管理 ===================== */

static VisitRecord *create_visit_record(int record_id, const char *diagnosis, double cost) {
    VisitRecord *r;

    r = (VisitRecord *)malloc(sizeof(VisitRecord));
    if (!r) {
        printf("【错误】内存分配失败！\n");
        return NULL;
    }
    r->recordId = record_id;
    strcpy(r->diagnosis, diagnosis);
    r->cost = cost;
    r->next = NULL;
    return r;
}

static void add_patient_visit(int patient_id) {
    Patient *p;
    char diag[DIAGNOSIS_LEN];
    double cost;
    VisitRecord *new_record;
    Doctor *doc;

    p = query_patient_by_id(patient_id);
    if (!p) {
        printf("【错误】患者不存在！无法添加病历\n解决方案：先挂号建档\n");
        return;
    }

    printf("请输入病情诊断：");
    if (fgets(diag, DIAGNOSIS_LEN, stdin) != NULL) {
        size_t len = strlen(diag);
        if (len > 0 && diag[len - 1] == '\n')
            diag[len - 1] = '\0';
        else
            while (getchar() != '\n' && getchar() != EOF);
    }
    if (strlen(diag) == 0) {
        printf("【错误】诊断不能为空！\n");
        return;
    }

    printf("请输入就诊费用：");
    while (scanf("%lf", &cost) != 1 || cost < 0) {
        printf("费用不能为负数！重新输入：");
        while (getchar() != '\n' && getchar() != EOF);
    }
    while (getchar() != '\n' && getchar() != EOF);

    new_record = create_visit_record(next_record_id++, diag, cost);
    if (!new_record) return;

    if (!p->records) {
        p->records = new_record;
    } else {
        VisitRecord *r = p->records;
        while (r->next) r = r->next;
        r->next = new_record;
    }
    p->totalCost += cost;

    doc = query_doctor_by_id(p->doctorId);
    if (doc) doc->patientCount++;

    printf("就诊病历添加成功！病历ID:%d 累计费用：%.2f\n", new_record->recordId, p->totalCost);
}

static void delete_patient_record(int pid) {
    Patient *p;
    int rid;
    VisitRecord *pre;
    VisitRecord *cur;

    p = query_patient_by_id(pid);
    if (!p) {
        printf("【错误】患者不存在！\n");
        return;
    }
    if (!p->records) {
        printf("【错误】该患者无病历可删除！\n");
        return;
    }

    printf("=== 该患者所有病历 ===\n");
    cur = p->records;
    while (cur) {
        printf("病历ID:%d 诊断:%s 费用:%.2f\n", cur->recordId, cur->diagnosis, cur->cost);
        cur = cur->next;
    }

    printf("请输入要删除的病历ID：");
    scanf("%d", &rid);
    while (getchar() != '\n' && getchar() != EOF);

    pre = NULL;
    cur = p->records;
    while (cur) {
        if (cur->recordId == rid) {
            p->totalCost -= cur->cost;
            if (!pre)
                p->records = cur->next;
            else
                pre->next = cur->next;
            free(cur);
            printf("病历删除成功，费用已扣除！剩余总费用：%.2f\n", p->totalCost);
            return;
        }
        pre = cur;
        cur = cur->next;
    }
    printf("【错误】未找到该病历ID！\n解决方案：先查询病历列表确认ID\n");
}

static void query_patient_record(int patient_id) {
    Patient *p;
    VisitRecord *r;

    p = query_patient_by_id(patient_id);
    if (!p) {
        printf("【错误】未找到该患者！\n");
        return;
    }
    printf("\n=== 患者【%s】病历记录 ===\n", p->name);
    if (!p->records) {
        printf("暂无就诊病历\n");
        return;
    }
    printf("%-8s %-20s %-10s\n", "病历ID", "诊断", "费用");
    printf("-----------------------------------\n");
    r = p->records;
    while (r) {
        printf("%-8d %-20s %-10.2f\n", r->recordId, r->diagnosis, r->cost);
        r = r->next;
    }
}

/* ===================== 检查项目管理 ===================== */

static void add_patient_check(int patient_id) {
    Patient *p;
    int type;
    char res[50];
    double fee;
    CheckRecord *cr;

    p = query_patient_by_id(patient_id);
    if (!p) {
        printf("【错误】患者不存在！无法开检查\n解决方案：先录入患者\n");
        return;
    }

    printf("\n===== 检查项目列表 =====\n");
    printf("1.血常规(45元)  2.尿常规(38元)  3.心电图(60元)\n");
    printf("4.胸部CT(280元)  5.腹部B超(120元)  6.核磁共振(500元)\n");
    printf("请选择检查编号：");
    scanf("%d", &type);
    while (getchar() != '\n' && getchar() != EOF);

    cr = (CheckRecord *)malloc(sizeof(CheckRecord));
    if (!cr) {
        printf("【错误】内存分配失败！\n");
        return;
    }
    cr->checkId = next_check_id++;
    cr->next = NULL;

    switch (type) {
        case 1: strcpy(cr->checkName, "血常规"); fee = 45; break;
        case 2: strcpy(cr->checkName, "尿常规"); fee = 38; break;
        case 3: strcpy(cr->checkName, "心电图"); fee = 60; break;
        case 4: strcpy(cr->checkName, "胸部CT"); fee = 280; break;
        case 5: strcpy(cr->checkName, "腹部B超"); fee = 120; break;
        case 6: strcpy(cr->checkName, "核磁共振"); fee = 500; break;
        default:
            printf("【错误】检查序号无效！\n解决方案：选择1-6之间数字\n");
            free(cr);
            return;
    }

    printf("请输入检查结果（正常/异常/阴性/阳性/待出）：");
    if (fgets(res, 50, stdin) != NULL) {
        size_t len = strlen(res);
        if (len > 0 && res[len - 1] == '\n')
            res[len - 1] = '\0';
        else
            while (getchar() != '\n' && getchar() != EOF);
    }
    strcpy(cr->checkResult, res);
    cr->checkFee = fee;
    p->totalCost += fee;

    if (!p->checks) {
        p->checks = cr;
    } else {
        CheckRecord *t = p->checks;
        while (t->next) t = t->next;
        t->next = cr;
    }
    printf("检查开具完成！检查ID:%d 项目:%s 费用%.2f元\n", cr->checkId, cr->checkName, fee);
}

static void modify_check_result(int pid) {
    Patient *p;
    int cid;
    char new_res[50];
    CheckRecord *t;

    p = query_patient_by_id(pid);
    if (!p) {
        printf("患者不存在！\n");
        return;
    }
    if (!p->checks) {
        printf("暂无检查记录，无法修改！\n");
        return;
    }

    printf("\n=== 该患者所有检查记录 ===\n");
    printf("%-8s %-12s %-15s\n", "检查ID", "项目", "当前结果");
    printf("---------------------------------\n");
    t = p->checks;
    while (t) {
        printf("%-8d %-12s %-15s\n", t->checkId, t->checkName, t->checkResult);
        t = t->next;
    }

    printf("请输入要修改的检查ID：");
    scanf("%d", &cid);
    while (getchar() != '\n' && getchar() != EOF);

    t = p->checks;
    while (t) {
        if (t->checkId == cid) {
            printf("请输入新的检查结果：");
            if (fgets(new_res, 50, stdin) != NULL) {
                size_t len = strlen(new_res);
                if (len > 0 && new_res[len - 1] == '\n')
                    new_res[len - 1] = '\0';
                else
                    while (getchar() != '\n' && getchar() != EOF);
            }
            strcpy(t->checkResult, new_res);
            printf("检查结果修改成功！\n");
            return;
        }
        t = t->next;
    }
    printf("未找到该检查ID！\n");
}

static void query_patient_check(int pid) {
    Patient *p;
    CheckRecord *t;

    p = query_patient_by_id(pid);
    if (!p) {
        printf("患者不存在！\n");
        return;
    }
    printf("\n=== 患者%s 检查报告明细 ===\n", p->name);
    if (!p->checks) {
        printf("暂无检查记录\n");
        return;
    }
    printf("%-8s %-12s %-15s %-10s\n", "检查ID", "项目", "结果", "费用");
    printf("------------------------------------------------\n");
    t = p->checks;
    while (t) {
        printf("%-8d %-12s %-15s %-10.2f\n",
               t->checkId, t->checkName, t->checkResult, t->checkFee);
        t = t->next;
    }
}

/* ===================== 处方管理（支持多药品） ===================== */

static void prescribe_medicine(int pid) {
    Patient *p;
    Prescription *pre;
    Medicine *m;
    char choice;
    int mid;
    int num;
    Medicine *med;
    PrescriptionItem *pi;
    PrescriptionItem *last;

    p = query_patient_by_id(pid);
    if (!p) {
        printf("【错误】患者不存在！无法开处方\n");
        return;
    }

    pre = (Prescription *)malloc(sizeof(Prescription));
    if (!pre) {
        printf("【错误】内存分配失败！\n");
        return;
    }
    pre->prescriptionId = next_prescription_id++;
    pre->patientId = pid;
    pre->totalCost = 0;
    pre->items = NULL;

    printf("\n=== 药品库存列表 ===\n");
    printf("%-8s %-16s %-10s %-8s %-10s\n", "药品ID", "名称", "单价", "库存", "预警线");
    printf("-------------------------------------------------\n");
    m = medicine_head;
    while (m) {
        printf("%-8d %-16s %-10.2f %-8d %-10d%s\n",
               m->medicineId, m->name, m->price, m->stock, m->warningLine,
               m->stock <= m->warningLine ? " [库存预警!]" : "");
        m = m->next;
    }

    do {
        printf("\n请输入药品ID（0结束添加）：");
        scanf("%d", &mid);
        while (getchar() != '\n' && getchar() != EOF);
        if (mid == 0) break;

        med = query_medicine_by_id(mid);
        if (!med) {
            printf("【错误】无此药品ID！\n");
            continue;
        }

        printf("请输入开药数量：");
        scanf("%d", &num);
        while (getchar() != '\n' && getchar() != EOF);

        if (num <= 0) {
            printf("【错误】数量不能<=0！\n");
            continue;
        }
        if (med->stock < num) {
            printf("【错误】药品[%s]库存不足！当前库存%d，需要%d\n",
                   med->name, med->stock, num);
            printf("解决方案：更换药品或减少数量\n");
            continue;
        }

        pi = (PrescriptionItem *)malloc(sizeof(PrescriptionItem));
        if (!pi) {
            printf("【错误】内存分配失败！\n");
            continue;
        }
        pi->medicineId = mid;
        strcpy(pi->medicineName, med->name);
        pi->quantity = num;
        pi->itemCost = med->price * num;
        pi->next = NULL;

        med->stock -= num;
        pre->totalCost += pi->itemCost;
        p->totalCost += pi->itemCost;

        if (!pre->items) {
            pre->items = pi;
        } else {
            last = pre->items;
            while (last->next) last = last->next;
            last->next = pi;
        }

        printf("已添加：%s x%d = %.2f元\n", med->name, num, pi->itemCost);
        printf("当前处方累计：%.2f元\n", pre->totalCost);

        if (med->stock <= med->warningLine) {
            printf("警告：[%s]库存已降至%d，低于预警线%d！\n",
                   med->name, med->stock, med->warningLine);
        }

        printf("是否继续添加药品？(y/n)：");
        scanf(" %c", &choice);
        while (getchar() != '\n' && getchar() != EOF);
    } while (choice == 'y' || choice == 'Y');

    if (!pre->items) {
        printf("处方为空，已取消\n");
        free(pre);
        return;
    }

    pre->next = prescription_head;
    prescription_head = pre;
    printf("\n处方开药成功！处方ID:%d 总费用%.2f元\n", pre->prescriptionId, pre->totalCost);
}

static void query_prescription(int pid) {
    Patient *p;
    Prescription *pr;
    int found;
    PrescriptionItem *pi;

    p = query_patient_by_id(pid);
    if (!p) {
        printf("患者不存在！\n");
        return;
    }

    printf("\n=== 患者%s 处方明细 ===\n", p->name);
    pr = prescription_head;
    found = 0;
    while (pr) {
        if (pr->patientId == pid) {
            found = 1;
            printf("\n处方ID:%d 总费用:%.2f元\n", pr->prescriptionId, pr->totalCost);
            printf("%-8s %-16s %-8s %-10s\n", "药品ID", "名称", "数量", "小计");
            printf("-----------------------------------------\n");
            pi = pr->items;
            while (pi) {
                printf("%-8d %-16s %-8d %-10.2f\n",
                       pi->medicineId, pi->medicineName, pi->quantity, pi->itemCost);
                pi = pi->next;
            }
        }
        pr = pr->next;
    }
    if (!found) printf("该患者暂无处方记录\n");
}

/* ===================== 候诊队列 ===================== */

static void query_all_queue(void) {
    QueueNode *t;
    char s[10];

    if (!queue_head) {
        printf("候诊队列为空\n");
        return;
    }

    printf("\n=== 候诊排队列表 ===\n");
    printf("%-8s %-8s %-8s %-8s %-10s\n", "队列号", "患者ID", "医生ID", "挂号ID", "状态");
    printf("------------------------------------------------\n");

    t = queue_head;
    while (t) {
        switch (t->status) {
            case 0: strcpy(s, "候诊"); break;
            case 1: strcpy(s, "已叫号"); break;
            case 2: strcpy(s, "就诊中"); break;
            case 3: strcpy(s, "已完成"); break;
            default: strcpy(s, "未知"); break;
        }
        printf("%-8d %-8d %-8d %-8d %-10s\n",
               t->queueId, t->patientId, t->doctorId, t->registerId, s);
        t = t->next;
    }
}

static void query_queue_by_doctor(void) {
    QueueNode *t;
    int found;
    int did;
    char s[10];

    if (!queue_head) {
        printf("候诊队列为空\n");
        return;
    }

    printf("\n===== 按医生查看候诊队列 =====\n");
    printf("请输入医生ID（0查看全部）：");
    scanf("%d", &did);
    while (getchar() != '\n' && getchar() != EOF);

    t = queue_head;
    found = 0;
    printf("%-8s %-8s %-8s %-10s\n", "队列号", "患者ID", "挂号ID", "状态");
    printf("---------------------------------------\n");
    while (t) {
        if (did == 0 || t->doctorId == did) {
            found = 1;
            switch (t->status) {
                case 0: strcpy(s, "候诊"); break;
                case 1: strcpy(s, "已叫号"); break;
                case 2: strcpy(s, "就诊中"); break;
                case 3: strcpy(s, "已完成"); break;
            }
            printf("%-8d %-8d %-8d %-10s\n", t->queueId, t->patientId, t->registerId, s);
        }
        t = t->next;
    }
    if (!found && did != 0) printf("该医生暂无候诊患者\n");
}

static void call_next_patient(void) {
    QueueNode *t;
    Patient *p;
    Doctor *d;

    if (!queue_head) {
        printf("【错误】无排队患者，无法叫号\n解决方案：先挂号\n");
        return;
    }

    t = queue_head;
    while (t && t->status != 0) t = t->next;

    if (!t) {
        printf("【提示】当前没有候诊状态的患者\n");
        return;
    }

    t->status = 2;
    p = query_patient_by_id(t->patientId);
    d = query_doctor_by_id(t->doctorId);
    printf("叫号成功！患者%s 进入就诊中（医生：%s）\n",
           p ? p->name : "未知", d ? d->name : "未知");
}

static void finish_visit(void) {
    QueueNode *t;
    Patient *p;

    if (!queue_head) {
        printf("暂无就诊患者\n");
        return;
    }

    t = queue_head;
    while (t && t->status != 2) t = t->next;

    if (!t) {
        printf("【提示】当前没有就诊中的患者\n");
        return;
    }

    t->status = 3;
    p = query_patient_by_id(t->patientId);
    printf("本次就诊接诊结束！患者：%s\n", p ? p->name : "未知");
}

/* ===================== 菜单与入口 ===================== */

static void show_menu(void) {
    printf("\n================== 医院门诊管理系统 ==================\n");
    printf("【科室与医生】\n");
    printf("  1.查询科室与医生    2.医生工作量统计\n");
    printf("【挂号候诊】\n");
    printf("  3.查看候诊队列      4.按医生查看队列   5.叫号就诊\n");
    printf("  6.结束本次就诊\n");
    printf("【诊疗服务】\n");
    printf("  7.添加就诊病历      8.删除就诊病历      9.查询病历\n");
    printf(" 10.开具检查项目     11.修改检查结果     12.查询检查报告\n");
    printf(" 13.开具处方药品     14.查询处方明细\n");
    printf("【住院与费用】\n");
    printf(" 15.设置住院/门诊\n");
    printf("【系统】\n");
    printf("  0.退出系统\n");
    printf("======================================================\n");
    printf("请输入操作序号：");
    fflush(stdout);
}

int B_entry(void) {
    int opt;
    int pid;

    printf("=== 医院管理系统 精简版 ===\n");

    if (!module_inited) init_base_data();

    while (1) {
        show_menu();
        while (scanf("%d", &opt) != 1) {
            printf("输入无效！请输入数字序号：");
            while (getchar() != '\n' && getchar() != EOF);
        }
        while (getchar() != '\n' && getchar() != EOF);

        switch (opt) {
            case 1: {
                char dept[DEPT_LEN];
                int num;

                query_all_department();
                if (dept_count == 0) break;
                printf("\n请输入要查看的科室名称或编号：");
                if (fgets(dept, DEPT_LEN, stdin) != NULL) {
                    size_t len = strlen(dept);
                    if (len > 0 && dept[len - 1] == '\n')
                        dept[len - 1] = '\0';
                    else
                        while (getchar() != '\n' && getchar() != EOF);
                }

                num = atoi(dept);
                if (num > 0 && num <= dept_count) {
                    strcpy(dept, dept_list[num - 1]);
                    printf("已选择科室：%s\n", dept);
                }
                query_doctor_by_dept(dept);
                break;
            }
            case 2:
                query_doctor_workload();
                break;
            case 3:
                query_all_queue();
                break;
            case 4:
                query_queue_by_doctor();
                break;
            case 5:
                call_next_patient();
                break;
            case 6:
                finish_visit();
                break;
            case 7:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                add_patient_visit(pid);
                break;
            case 8:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                delete_patient_record(pid);
                break;
            case 9:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                query_patient_record(pid);
                break;
            case 10:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                add_patient_check(pid);
                break;
            case 11:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                modify_check_result(pid);
                break;
            case 12:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                query_patient_check(pid);
                break;
            case 13:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                prescribe_medicine(pid);
                break;
            case 14:
                printf("患者ID：");
                scanf("%d", &pid);
                while (getchar() != '\n' && getchar() != EOF);
                query_prescription(pid);
                break;
            case 15:
                change_patient_hospital();
                break;
            case 0:
                free_all_memory();
                printf("感谢使用，再见！\n");
                return 0;
            default:
                printf("【错误】菜单序号超出范围！\n解决方案：选择0~15正确选项\n");
                break;
        }

        if (opt != 0) {
            printf("\n按 Enter 键继续...");
            while (getchar() != '\n' && getchar() != EOF);
            printf("\n\n");
            {
                int i;
                for (i = 0; i < 50; i++) printf("\n");
            }
        }
    }
}

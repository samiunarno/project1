#include "common.h"
#include "utils.h"

/*
 * A 模块：患者 / 挂号 / 初始化
 * 只保留 A 自己的代码，不掺 B / C / D 的业务逻辑。
 */

static Patient *g_patients = NULL;
static Registration *g_regs = NULL;
static int g_nextPatientId = PATIENT_ID_START;
static int g_nextRegisterId = REGISTER_ID_START;
static int g_nextRegisterTime = REGISTER_TIME_START;
static int g_nextRecordId = RECORD_ID_START;
static int g_a_inited = 0;

static int input_int_with_back_a(const char *prompt, int min, int max, int *back) {
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

static int input_string_with_back_a(const char *prompt, char *buffer, int size) {
    utils_input_string(prompt, buffer, size);
    return strcmp(buffer, "0") != 0;
}

/* ===================== 患者与就诊记录 ===================== */

static Patient *patient_create_a(int patientId, const char *name, int age,
                                 const char *gender, const char *department) {
    Patient *p = (Patient *)malloc(sizeof(Patient));
    if (!p) return NULL;
    p->patientId = patientId;
    strncpy(p->name, name, NAME_LEN - 1);
    p->name[NAME_LEN - 1] = '\0';
    p->age = age;
    strncpy(p->gender, gender, GENDER_LEN - 1);
    p->gender[GENDER_LEN - 1] = '\0';
    strncpy(p->department, department, DEPT_LEN - 1);
    p->department[DEPT_LEN - 1] = '\0';
    p->doctorId = -1;
    p->isHospitalized = 0;
    p->bedId = -1;
    p->totalCost = 0.0f;
    p->records = NULL;
    p->next = NULL;
    return p;
}

static int patient_add_a(Patient **head, Patient *node) {
    if (!head || !node) return 0;
    if (*head == NULL) {
        *head = node;
        return 1;
    }
    Patient *cur = *head;
    while (cur->next) cur = cur->next;
    cur->next = node;
    return 1;
}

static Patient *patient_find_by_id_a(Patient *head, int patientId) {
    while (head) {
        if (head->patientId == patientId) return head;
        head = head->next;
    }
    return NULL;
}

static void patient_find_by_name_a(Patient *head, const char *name) {
    int found = 0;
    while (head) {
        if (strcmp(head->name, name) == 0) {
            printf("患者编号：%d\n", head->patientId);
            printf("姓名：%s\n", head->name);
            printf("年龄：%d\n", head->age);
            printf("性别：%s\n", head->gender);
            printf("科室：%s\n", head->department);
            printf("医生编号：%d\n", head->doctorId);
            ui_print_line();
            found = 1;
        }
        head = head->next;
    }
    if (!found) printf("未找到对应患者。\n");
}

static void patient_find_by_department_a(Patient *head, const char *department) {
    int found = 0;
    while (head) {
        if (strcmp(head->department, department) == 0) {
            printf("患者编号：%d\n", head->patientId);
            printf("姓名：%s\n", head->name);
            printf("年龄：%d\n", head->age);
            printf("性别：%s\n", head->gender);
            printf("科室：%s\n", head->department);
            printf("医生编号：%d\n", head->doctorId);
            ui_print_line();
            found = 1;
        }
        head = head->next;
    }
    if (!found) printf("该科室下暂无患者。\n");
}

static VisitRecord *record_create_a(int recordId, const char *diagnosis, float cost) {
    VisitRecord *r = (VisitRecord *)malloc(sizeof(VisitRecord));
    if (!r) return NULL;
    r->recordId = recordId;
    strncpy(r->diagnosis, diagnosis, DIAGNOSIS_LEN - 1);
    r->diagnosis[DIAGNOSIS_LEN - 1] = '\0';
    r->cost = cost;
    r->next = NULL;
    return r;
}

static int patient_add_record_a(Patient *patient, VisitRecord *record) {
    if (!patient || !record) return 0;
    if (!patient->records) {
        patient->records = record;
    } else {
        VisitRecord *cur = patient->records;
        while (cur->next) cur = cur->next;
        cur->next = record;
    }
    patient->totalCost += record->cost;
    return 1;
}

static void patient_add_record_flow_a(Patient *head, int *nextRecordId) {
    int step = 0;
    int patientId = 0, back = 0;
    char diagnosis[DIAGNOSIS_LEN] = "";
    float cost = 0.0f;
    Patient *p = NULL;
    VisitRecord *record = NULL;

    while (1) {
        if (step == 0) {
            patientId = input_int_with_back_a("请输入患者编号：", 1, 999999, &back);
            if (back) return;
            p = patient_find_by_id_a(head, patientId);
            if (!p) {
                printf("未找到对应患者。\n");
                continue;
            }
            step = 1;
        } else if (step == 1) {
            if (!input_string_with_back_a("请输入诊断结果：", diagnosis, sizeof(diagnosis))) {
                step = 0;
                continue;
            }
            step = 2;
        } else {
            cost = utils_input_float("请输入本次费用：", 0.0f, 1000000.0f);
            if (cost == 0.0f) {
                step = 1;
                continue;
            }
            record = record_create_a((*nextRecordId)++, diagnosis, cost);
            if (!record || !patient_add_record_a(p, record)) {
                printf("追加就诊记录失败。\n");
                free(record);
                return;
            }
            printf("追加就诊记录成功！记录编号：%d\n", record->recordId);
            return;
        }
    }
}

static void patient_show_records_a(const Patient *patient) {
    const VisitRecord *cur;
    if (!patient) return;
    ui_print_sub_title("就诊记录");
    if (!patient->records) {
        printf("该患者暂无就诊记录。\n");
        return;
    }
    cur = patient->records;
    while (cur) {
        printf("记录编号：%d | 诊断：%s | 本次费用：%.2f\n", cur->recordId, cur->diagnosis, cur->cost);
        cur = cur->next;
    }
}

static void patient_display_one_a(const Patient *p) {
    if (!p) return;
    printf("患者编号：%d\n", p->patientId);
    printf("姓名：%s\n", p->name);
    printf("年龄：%d\n", p->age);
    printf("性别：%s\n", p->gender);
    printf("科室：%s\n", p->department);
    printf("医生编号：%d\n", p->doctorId);
    printf("是否住院：%s\n", p->isHospitalized ? "是" : "否");
    printf("床位编号：%d\n", p->bedId);
    printf("总费用：%.2f\n", p->totalCost);
}

static void patient_display_all_a(Patient *head) {
    ui_print_sub_title("患者列表");
    if (!head) {
        printf("暂无患者信息。\n");
        return;
    }
    printf("%-8s %-10s %-6s %-6s %-8s %-8s\n", "编号", "姓名", "年龄", "性别", "科室", "医生");
    ui_print_line();
    while (head) {
        printf("%-8d %-10s %-6d %-6s %-8s %-8d\n", head->patientId, head->name, head->age,
               head->gender, head->department, head->doctorId);
        head = head->next;
    }
}

/* 返回值：1成功；0未找到患者；-1表示回到上一个输入项 */
static int patient_update_a(Patient *head, int patientId) {
    char name[NAME_LEN], gender[GENDER_LEN], dept[DEPT_LEN];
    int age = 0, back = 0, step = 0;
    Patient *p = patient_find_by_id_a(head, patientId);
    if (!p) return 0;

    while (1) {
        if (step == 0) {
            if (!input_string_with_back_a("请输入姓名：", name, sizeof(name))) return -1;
            step = 1;
        } else if (step == 1) {
            age = input_int_with_back_a("请输入年龄：", 1, 150, &back);
            if (back) {
                step = 0;
                continue;
            }
            step = 2;
        } else if (step == 2) {
            if (!input_string_with_back_a("请输入性别：", gender, sizeof(gender))) {
                step = 1;
                continue;
            }
            step = 3;
        } else {
            if (!input_string_with_back_a("请输入科室：", dept, sizeof(dept))) {
                step = 2;
                continue;
            }
            strncpy(p->name, name, NAME_LEN - 1); p->name[NAME_LEN - 1] = '\0';
            strncpy(p->gender, gender, GENDER_LEN - 1); p->gender[GENDER_LEN - 1] = '\0';
            strncpy(p->department, dept, DEPT_LEN - 1); p->department[DEPT_LEN - 1] = '\0';
            p->age = age;
            return 1;
        }
    }
}

static void free_records_a(VisitRecord *head) {
    while (head) {
        VisitRecord *tmp = head;
        head = head->next;
        free(tmp);
    }
}

static int patient_delete_a(Patient **head, int patientId) {
    Patient *cur, *prev;
    if (!head || !*head) return 0;
    cur = *head;
    prev = NULL;
    while (cur) {
        if (cur->patientId == patientId) {
            if (prev) prev->next = cur->next;
            else *head = cur->next;
            free_records_a(cur->records);
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

static void patient_register_a(Patient **head, int *nextPatientId) {
    char name[NAME_LEN], gender[GENDER_LEN], dept[DEPT_LEN];
    int age = 0, back = 0, step = 0;

    while (1) {
        if (step == 0) {
            if (!input_string_with_back_a("请输入姓名：", name, sizeof(name))) return;
            step = 1;
        } else if (step == 1) {
            age = input_int_with_back_a("请输入年龄：", 1, 150, &back);
            if (back) {
                step = 0;
                continue;
            }
            step = 2;
        } else if (step == 2) {
            if (!input_string_with_back_a("请输入性别：", gender, sizeof(gender))) {
                step = 1;
                continue;
            }
            step = 3;
        } else {
            if (!input_string_with_back_a("请输入科室：", dept, sizeof(dept))) {
                step = 2;
                continue;
            }
            Patient *p = patient_create_a((*nextPatientId)++, name, age, gender, dept);
            if (!p || !patient_add_a(head, p)) {
                printf("新增患者失败。\n");
                free(p);
                return;
            }
            printf("新增患者成功！患者编号为：%d\n", p->patientId);
            return;
        }
    }
}

/* ===================== 挂号记录 ===================== */

static const char *register_type_text_a(int type) {
    return type == REGISTER_BOOK ? "预约挂号" : "现场挂号";
}

static const char *register_status_text_a(int status) {
    switch (status) {
        case STATUS_WAIT: return "候诊中";
        case STATUS_CALL: return "已叫号";
        case STATUS_TREATING: return "就诊中";
        case STATUS_FINISH: return "已完成";
        default: return "未知";
    }
}

static Registration *registration_create_a(int registerId, int patientId, int doctorId,
                                           const char *department, int registerType,
                                           int registerTime) {
    Registration *r = (Registration *)malloc(sizeof(Registration));
    if (!r) return NULL;
    r->registerId = registerId;
    r->patientId = patientId;
    r->doctorId = doctorId;
    strncpy(r->department, department, DEPT_LEN - 1);
    r->department[DEPT_LEN - 1] = '\0';
    r->registerType = registerType;
    r->registerTime = registerTime;
    r->status = STATUS_WAIT;
    r->next = NULL;
    return r;
}

static int registration_add_a(Registration **head, Registration *node) {
    if (!head || !node) return 0;
    if (*head == NULL) {
        *head = node;
        return 1;
    }
    Registration *cur = *head;
    while (cur->next) cur = cur->next;
    cur->next = node;
    return 1;
}

static Registration *registration_find_by_id_a(Registration *head, int registerId) {
    while (head) {
        if (head->registerId == registerId) return head;
        head = head->next;
    }
    return NULL;
}

static void registration_display_one_a(const Registration *r) {
    if (!r) return;
    printf("挂号编号：%d\n", r->registerId);
    printf("患者编号：%d\n", r->patientId);
    printf("医生编号：%d\n", r->doctorId);
    printf("科室：%s\n", r->department);
    printf("挂号方式：%s\n", register_type_text_a(r->registerType));
    printf("挂号顺序：%d\n", r->registerTime);
    printf("当前状态：%s\n", register_status_text_a(r->status));
}

static void registration_display_all_a(Registration *head) {
    ui_print_sub_title("全部挂号记录");
    if (!head) {
        printf("暂无挂号记录。\n");
        return;
    }
    printf("%-8s %-8s %-8s %-8s %-10s %-10s %-10s\n", "挂号号", "患者", "医生", "科室", "挂号方式", "顺序号", "状态");
    ui_print_line();
    while (head) {
        printf("%-8d %-8d %-8d %-8s %-10s %-10d %-10s\n", head->registerId, head->patientId, head->doctorId,
               head->department, register_type_text_a(head->registerType),
               head->registerTime, register_status_text_a(head->status));
        head = head->next;
    }
}

static void registration_query_by_patient_a(Registration *head, int patientId) {
    int found = 0;
    while (head) {
        if (head->patientId == patientId) {
            registration_display_one_a(head);
            ui_print_line();
            found = 1;
        }
        head = head->next;
    }
    if (!found) printf("该患者暂无挂号记录。\n");
}

static void registration_query_menu_a(Registration *regHead) {
    int choice;
    while (1) {
        ui_print_sub_title("挂号记录查询");
        printf("1. 按挂号编号查询\n");
        printf("2. 按患者编号查询\n");
        printf("3. 显示全部挂号记录\n");
        printf("0. 返回上一级\n");
        printf("提示：输入 0 可返回上一项。\n");
        ui_print_line();
        choice = utils_input_int("请输入选项：", 0, 3);
        if (choice == 0) break;

        if (choice == 1) {
            int rid, back = 0;
            rid = input_int_with_back_a("请输入挂号编号：", 1, 999999, &back);
            if (back) continue;
            Registration *r = registration_find_by_id_a(regHead, rid);
            if (r) registration_display_one_a(r);
            else printf("未找到对应挂号记录。\n");
        } else if (choice == 2) {
            int pid, back = 0;
            pid = input_int_with_back_a("请输入患者编号：", 1, 999999, &back);
            if (back) continue;
            registration_query_by_patient_a(regHead, pid);
        } else {
            registration_display_all_a(regHead);
        }
        utils_wait_enter();
    }
}

static void registration_add_flow_a(Patient *pHead, Registration **regHead,
                                    int *nextRegisterId, int *nextRegisterTime,
                                    int registerType) {
    int step = 0, patientId = 0, doctorId = 0, back = 0;
    Patient *p = NULL;

    while (1) {
        if (step == 0) {
            patientId = input_int_with_back_a("请输入患者编号：", 1, 999999, &back);
            if (back) return;
            p = patient_find_by_id_a(pHead, patientId);
            if (!p) {
                printf("患者不存在，无法挂号。\n");
                continue;
            }
            step = 1;
        } else {
            doctorId = input_int_with_back_a("请输入医生编号：", 1, 999999, &back);
            if (back) {
                step = 0;
                continue;
            }
            Registration *r = registration_create_a((*nextRegisterId)++, patientId, doctorId,
                                                    p->department, registerType,
                                                    (*nextRegisterTime)++);
            if (!r || !registration_add_a(regHead, r)) {
                printf("挂号失败。\n");
                free(r);
                return;
            }
            p->doctorId = doctorId;
            printf("%s成功！挂号编号：%d\n", register_type_text_a(registerType), r->registerId);
            return;
        }
    }
}

/* ===================== 初始化 ===================== */

static void A_init_once(void) {
    if (g_a_inited) return;

    Patient *p;
    p = patient_create_a(g_nextPatientId++, "张三", 20, "男", "内科");
    if (p) {
        patient_add_record_a(p, record_create_a(g_nextRecordId++, "普通感冒", 35.0f));
        patient_add_a(&g_patients, p);
    }

    p = patient_create_a(g_nextPatientId++, "李四", 31, "男", "外科");
    if (p) patient_add_a(&g_patients, p);

    p = patient_create_a(g_nextPatientId++, "王五", 8, "女", "儿科");
    if (p) patient_add_a(&g_patients, p);

    g_a_inited = 1;
}

/* ===================== A 模块入口 ===================== */

int A_entry(void) {
    int choice;
    A_init_once();

    while (1) {
        ui_print_sub_title("A 模块：患者 / 挂号 / 初始化");
        printf("1. 新增患者\n");
        printf("2. 按编号查询患者\n");
        printf("3. 按姓名查询患者\n");
        printf("4. 按科室查询患者\n");
        printf("5. 修改患者信息\n");
        printf("6. 删除患者信息\n");
        printf("7. 显示所有患者\n");
        printf("8. 预约挂号\n");
        printf("9. 现场挂号\n");
        printf("10. 查询挂号记录\n");
        printf("11. 追加就诊记录\n");
        printf("12. 查看患者就诊记录\n");
        printf("0. 返回主菜单\n");
        printf("提示：输入 0 可返回上一项。\n");
        ui_print_line();

        choice = utils_input_int("请输入选项：", 0, 12);
        if (choice == 0) return 0;

        switch (choice) {
            case 1:
                patient_register_a(&g_patients, &g_nextPatientId);
                break;
            case 2: {
                int id, back = 0;
                id = input_int_with_back_a("请输入患者编号：", 1, 999999, &back);
                if (back) break;
                Patient *p = patient_find_by_id_a(g_patients, id);
                if (p) patient_display_one_a(p);
                else printf("未找到对应患者。\n");
                break;
            }
            case 3: {
                char name[NAME_LEN];
                if (!input_string_with_back_a("请输入姓名：", name, sizeof(name))) break;
                patient_find_by_name_a(g_patients, name);
                break;
            }
            case 4: {
                char dept[DEPT_LEN];
                if (!input_string_with_back_a("请输入科室：", dept, sizeof(dept))) break;
                patient_find_by_department_a(g_patients, dept);
                break;
            }
            case 5: {
                int id, back = 0, ret;
                id = input_int_with_back_a("请输入要修改的患者编号：", 1, 999999, &back);
                if (back) break;
                ret = patient_update_a(g_patients, id);
                if (ret == 1) printf("修改成功。\n");
                else if (ret == 0) printf("未找到对应患者。\n");
                break;
            }
            case 6: {
                int id, back = 0;
                id = input_int_with_back_a("请输入要删除的患者编号：", 1, 999999, &back);
                if (back) break;
                if (patient_delete_a(&g_patients, id)) printf("删除成功。\n");
                else printf("未找到对应患者。\n");
                break;
            }
            case 7:
                patient_display_all_a(g_patients);
                break;
            case 8:
                registration_add_flow_a(g_patients, &g_regs, &g_nextRegisterId, &g_nextRegisterTime, REGISTER_BOOK);
                break;
            case 9:
                registration_add_flow_a(g_patients, &g_regs, &g_nextRegisterId, &g_nextRegisterTime, REGISTER_ON_SITE);
                break;
            case 10:
                registration_query_menu_a(g_regs);
                break;
            case 11:
                patient_add_record_flow_a(g_patients, &g_nextRecordId);
                break;
            case 12: {
                int id, back = 0;
                id = input_int_with_back_a("请输入患者编号：", 1, 999999, &back);
                if (back) break;
                Patient *p = patient_find_by_id_a(g_patients, id);
                if (p) patient_show_records_a(p);
                else printf("未找到对应患者。\n");
                break;
            }
            default:
                printf("无效选项。\n");
                break;
        }
        utils_wait_enter();
    }
}

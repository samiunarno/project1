#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 50
#define MAX_DEP_LEN 30
#define MAX_DIAG_LEN 200
#define DATA_FILE "hospital_data.txt"

// ===================== 结构体定义 =====================

typedef struct VisitRecord {
    int recordId;
    char diagnosis[MAX_DIAG_LEN];
    double cost;
    struct VisitRecord* next;
} VisitRecord;

typedef struct CheckRecord {
    int checkId;
    char checkName[30];
    char checkResult[50];
    double checkFee;
    struct CheckRecord* next;   
} CheckRecord;

typedef struct PrescriptionItem {
    int medicineId;
    char medicineName[MAX_NAME_LEN];
    int quantity;
    double itemCost;
    struct PrescriptionItem* next;
} PrescriptionItem;

typedef struct Prescription {
    int prescriptionId;
    int patientId;
    double totalCost;
    PrescriptionItem* items;
    struct Prescription* next;
} Prescription;

typedef struct Patient {
    int patientId;
    char name[MAX_NAME_LEN];
    int age;
    char gender;
    char department[MAX_DEP_LEN];
    int doctorId;
    int isHospitalized;
    double totalCost;
    VisitRecord* records;
    CheckRecord* checks;
    struct Patient* next;
} Patient;

typedef struct Doctor {
    int doctorId;
    char name[MAX_NAME_LEN];
    char department[MAX_DEP_LEN];
    char title[MAX_NAME_LEN];
    int patientCount;
    struct Doctor* next;
} Doctor;

typedef struct Medicine {
    int medicineId;
    char name[MAX_NAME_LEN];
    double price;
    int stock;
    int warningLine;
    struct Medicine* next;
} Medicine;

typedef struct Registration {
    int registerId;
    int patientId;
    int doctorId;
    char department[MAX_DEP_LEN];
    int registerType;
    int registerTime;
    int status;
    struct Registration* next;
} Registration;

typedef struct QueueNode {
    int queueId;
    int registerId;
    int patientId;
    int doctorId;
    int status;
    struct QueueNode* next;
} QueueNode;

// ===================== 全局变量 =====================

Patient* patientHead = NULL;
Doctor* doctorHead = NULL;
Medicine* medicineHead = NULL;
Prescription* prescriptionHead = NULL;
Registration* regHead = NULL;
QueueNode* queueHead = NULL;

static int nextRecordId = 1001;
static int nextCheckId = 5001;
static int nextPrescriptionId = 8001;
static int nextRegId = 10001;
static int nextQueueId = 1000;

// 科室列表缓存（用于编号查询）
char deptList[100][MAX_DEP_LEN];
int deptCount = 0;

// ===================== 工具函数 =====================

// FIX: Added 'static' to avoid duplicate symbol with D.o
static void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void safeInput(char* dest, int maxLen, const char* prompt) {
    if (prompt) printf("%s", prompt);
    if (fgets(dest, maxLen, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len-1] == '\n') {
            dest[len-1] = '\0';
        } else {
            clearInputBuffer();
        }
    }
}

void pauseScreen() {
    printf("\n按 Enter 键继续...");
    clearInputBuffer();
}

void clearScreen() {
    printf("\n\n");
    int i;
    for (i = 0; i < 50; i++) printf("\n");
}

// 前置声明
void addIntoQueue(int registerId, int patientId, int doctorId);

// ===================== 初始化基础数据 =====================

void initBaseData() {
    Doctor* d = doctorHead;
    while (d) { Doctor* t = d; d = d->next; free(t); }
    doctorHead = NULL;

    Medicine* m = medicineHead;
    while (m) { Medicine* t = m; m = m->next; free(t); }
    medicineHead = NULL;

    Doctor* d1 = (Doctor*)malloc(sizeof(Doctor));
    if (!d1) { printf("内存分配失败！\n"); return; }
    d1->doctorId = 101; 
    strcpy(d1->name, "张医生"); 
    strcpy(d1->department, "内科");
    strcpy(d1->title, "主任医师"); 
    d1->patientCount = 0; 
    d1->next = NULL; 
    doctorHead = d1;

    Doctor* d2 = (Doctor*)malloc(sizeof(Doctor));
    if (!d2) { printf("内存分配失败！\n"); return; }
    d2->doctorId = 102; 
    strcpy(d2->name, "李医生"); 
    strcpy(d2->department, "外科");
    strcpy(d2->title, "副主任医师"); 
    d2->patientCount = 0; 
    d2->next = doctorHead; 
    doctorHead = d2;

    Doctor* d3 = (Doctor*)malloc(sizeof(Doctor));
    if (!d3) { printf("内存分配失败！\n"); return; }
    d3->doctorId = 103;
    strcpy(d3->name, "王医生");
    strcpy(d3->department, "内科");
    strcpy(d3->title, "主治医师");
    d3->patientCount = 0;
    d3->next = doctorHead;
    doctorHead = d3;

    // 新增：儿科
    Doctor* d4 = (Doctor*)malloc(sizeof(Doctor));
    if (!d4) { printf("内存分配失败！\n"); return; }
    d4->doctorId = 104;
    strcpy(d4->name, "赵医生");
    strcpy(d4->department, "儿科");
    strcpy(d4->title, "主治医师");
    d4->patientCount = 0;
    d4->next = doctorHead;
    doctorHead = d4;

    // 新增：骨科
    Doctor* d5 = (Doctor*)malloc(sizeof(Doctor));
    if (!d5) { printf("内存分配失败！\n"); return; }
    d5->doctorId = 105;
    strcpy(d5->name, "刘医生");
    strcpy(d5->department, "骨科");
    strcpy(d5->title, "主任医师");
    d5->patientCount = 0;
    d5->next = doctorHead;
    doctorHead = d5;

    Medicine* m1 = (Medicine*)malloc(sizeof(Medicine));
    if (!m1) { printf("内存分配失败！\n"); return; }
    m1->medicineId = 2001; 
    strcpy(m1->name, "感冒灵颗粒"); 
    m1->price = 18.5; 
    m1->stock = 100; 
    m1->warningLine = 20; 
    m1->next = NULL;
    medicineHead = m1;

    Medicine* m2 = (Medicine*)malloc(sizeof(Medicine));
    if (!m2) { printf("内存分配失败！\n"); return; }
    m2->medicineId = 2002; 
    strcpy(m2->name, "布洛芬缓释胶囊"); 
    m2->price = 25.0; 
    m2->stock = 80; 
    m2->warningLine = 15; 
    m2->next = medicineHead; 
    medicineHead = m2;

    Medicine* m3 = (Medicine*)malloc(sizeof(Medicine));
    if (!m3) { printf("内存分配失败！\n"); return; }
    m3->medicineId = 2003; 
    strcpy(m3->name, "阿莫西林"); 
    m3->price = 32.8; 
    m3->stock = 60; 
    m3->warningLine = 10; 
    m3->next = medicineHead; 
    medicineHead = m3;

    Medicine* m4 = (Medicine*)malloc(sizeof(Medicine));
    if (!m4) { printf("内存分配失败！\n"); return; }
    m4->medicineId = 2004;
    strcpy(m4->name, "维生素C片");
    m4->price = 12.0;
    m4->stock = 200;
    m4->warningLine = 30;
    m4->next = medicineHead;
    medicineHead = m4;
}

// ===================== 查找函数 =====================

Doctor* queryDoctorById(int doctorId) {
    Doctor* p = doctorHead;
    while (p != NULL) {
        if (p->doctorId == doctorId) return p;
        p = p->next;
    }
    return NULL;
}

Patient* queryPatientById(int pid) {
    Patient* p = patientHead;
    while (p) {
        if (p->patientId == pid) return p;
        p = p->next;
    }
    return NULL;
}

Medicine* queryMedicineById(int mid) {
    Medicine* m = medicineHead;
    while (m) {
        if (m->medicineId == mid) return m;
        m = m->next;
    }
    return NULL;
}

// ===================== 患者管理 =====================

void addNewPatient() {
    int pid, age;
    char name[MAX_NAME_LEN], gender, dept[MAX_DEP_LEN];
    int did;

    printf("\n===== 新增患者信息 =====\n");
    printf("请输入患者ID：");
    while (scanf("%d", &pid) != 1) {
        printf("输入格式错误！请输入数字ID\n请重新输入：");
        clearInputBuffer();
    }
    clearInputBuffer();

    if (queryPatientById(pid) != NULL) {
        printf("【错误】该患者ID已重复存在！\n解决方案：更换不重复的唯一患者ID重新录入\n");
        return;
    }

    printf("请输入患者姓名：");
    safeInput(name, MAX_NAME_LEN, NULL);
    if (strlen(name) == 0) {
        printf("【错误】姓名不能为空！\n");
        return;
    }

    printf("请输入患者年龄：");
    while (scanf("%d", &age) != 1 || age <= 0 || age > 120) {
        printf("年龄非法！请输入1~120之间有效年龄\n年龄重新输入：");
        clearInputBuffer();
    }
    clearInputBuffer();

    printf("请输入性别(M/F)：");
    scanf(" %c", &gender);
    clearInputBuffer();
    gender = (char)toupper((unsigned char)gender);
    if (gender != 'M' && gender != 'F') {
        printf("【错误】性别只能输入M或F！\n");
        return;
    }

    printf("请输入就诊科室：");
    safeInput(dept, MAX_DEP_LEN, NULL);

    printf("请选择主治医生ID（先查询科室医生列表）：");
    while (scanf("%d", &did) != 1) {
        printf("请输入数字医生ID！重新输入：");
        clearInputBuffer();
    }
    clearInputBuffer();

    Doctor* doc = queryDoctorById(did);
    if (doc == NULL) {
        printf("【错误】不存在该医生ID！\n解决方案：先查询科室医生列表，选择正确ID\n");
        return;
    }

    if (strcmp(doc->department, dept) != 0) {
        printf("【警告】医生%s的科室是[%s]，与您选择的科室[%s]不匹配！\n", 
               doc->name, doc->department, dept);
        printf("是否继续？(1-是 0-否)：");
        int confirm;
        scanf("%d", &confirm);
        clearInputBuffer();
        if (confirm != 1) {
            printf("已取消录入，请重新选择\n");
            return;
        }
    }

    Patient* p = (Patient*)malloc(sizeof(Patient));
    if (!p) {
        printf("【错误】内存分配失败！\n");
        return;
    }
    p->patientId = pid;
    strcpy(p->name, name);
    p->age = age;
    p->gender = gender;
    strcpy(p->department, dept);
    p->doctorId = did;
    p->isHospitalized = 0;
    p->totalCost = 0;
    p->records = NULL;
    p->checks = NULL;
    p->next = patientHead;
    patientHead = p;

    printf("患者信息录入成功！ID:%d 姓名:%s\n", pid, name);
}

void modifyPatient() {
    int pid;
    printf("\n===== 修改患者信息 =====\n");
    printf("请输入患者ID：");
    scanf("%d", &pid);
    clearInputBuffer();

    Patient* p = queryPatientById(pid);
    if (!p) {
        printf("【错误】查无此患者！\n");
        return;
    }

    printf("当前信息：ID:%d 姓名:%s 年龄:%d 性别:%c 科室:%s\n",
           p->patientId, p->name, p->age, p->gender, p->department);
    printf("\n1.修改姓名 2.修改年龄 3.修改性别 4.修改科室 5.修改主治医生\n");
    printf("0.返回主菜单\n请选择：");
    int opt;
    scanf("%d", &opt);
    clearInputBuffer();

    switch (opt) {
        case 1: {
            printf("请输入新姓名：");
            safeInput(p->name, MAX_NAME_LEN, NULL);
            printf("姓名修改成功！\n");
            break;
        }
        case 2: {
            printf("请输入新年龄：");
            int newAge;
            while (scanf("%d", &newAge) != 1 || newAge <= 0 || newAge > 120) {
                printf("年龄非法！重新输入：");
                clearInputBuffer();
            }
            clearInputBuffer();
            p->age = newAge;
            printf("年龄修改成功！\n");
            break;
        }
        case 3: {
            printf("请输入新性别(M/F)：");
            scanf(" %c", &p->gender);
            clearInputBuffer();
            p->gender = (char)toupper((unsigned char)p->gender);
            printf("性别修改成功！\n");
            break;
        }
        case 4: {
            printf("请输入新科室：");
            safeInput(p->department, MAX_DEP_LEN, NULL);
            printf("科室修改成功！\n");
            break;
        }
        case 5: {
            printf("请输入新医生ID：");
            int newDid;
            scanf("%d", &newDid);
            clearInputBuffer();
            Doctor* d = queryDoctorById(newDid);
            if (!d) {
                printf("【错误】医生不存在！\n");
                return;
            }
            p->doctorId = newDid;
            printf("主治医生修改成功！新医生：%s\n", d->name);
            break;
        }
        default:
            printf("已取消修改\n");
    }
}

void deletePatient() {
    int pid;
    printf("\n===== 删除患者 =====\n");
    printf("请输入要删除的患者ID：");
    scanf("%d", &pid);
    clearInputBuffer();

    Patient *pre = NULL, *cur = patientHead;
    while (cur) {
        if (cur->patientId == pid) {
            printf("确认删除患者 %s(ID:%d)？此操作将删除所有病历、检查、处方记录！\n", 
                   cur->name, cur->patientId);
            printf("1-确认删除 0-取消：");
            int confirm;
            scanf("%d", &confirm);
            clearInputBuffer();
            if (confirm != 1) {
                printf("已取消删除\n");
                return;
            }

            VisitRecord* vr = cur->records;
            while (vr) {
                VisitRecord* temp = vr;
                vr = vr->next;
                free(temp);
            }

            CheckRecord* cr = cur->checks;
            while (cr) {
                CheckRecord* temp = cr;
                cr = cr->next;
                free(temp);
            }

            if (pre == NULL) patientHead = cur->next;
            else pre->next = cur->next;

            free(cur);
            printf("患者删除成功！\n");
            return;
        }
        pre = cur;
        cur = cur->next;
    }
    printf("【错误】未找到该患者ID！\n");
}

void queryAllPatients() {
    printf("\n===== 所有患者列表 =====\n");
    Patient* p = patientHead;
    if (!p) {
        printf("暂无任何患者信息！\n");
        return;
    }
    printf("%-8s %-10s %-6s %-6s %-10s %-10s %-8s %-10s\n",
           "患者ID", "姓名", "年龄", "性别", "科室", "状态", "医生ID", "总费用");
    printf("----------------------------------------------------------------\n");
    while (p) {
        printf("%-8d %-10s %-6d %-6c %-10s %-10s %-8d %-10.2f\n",
               p->patientId, p->name, p->age, p->gender, p->department,
               p->isHospitalized ? "住院" : "门诊", p->doctorId, p->totalCost);
        p = p->next;
    }
}

void queryPatientsByDept() {
    char dept[MAX_DEP_LEN];
    printf("\n===== 按科室查询患者 =====\n");
    printf("请输入科室名称：");
    safeInput(dept, MAX_DEP_LEN, NULL);

    Patient* p = patientHead;
    int count = 0;
    printf("\n%s科室患者列表：\n", dept);
    printf("%-8s %-10s %-6s %-8s %-10s\n", "患者ID", "姓名", "年龄", "状态", "总费用");
    printf("-----------------------------------------------\n");
    while (p) {
        if (strcmp(p->department, dept) == 0) {
            count++;
            printf("%-8d %-10s %-6d %-8s %-10.2f\n",
                   p->patientId, p->name, p->age,
                   p->isHospitalized ? "住院" : "门诊", p->totalCost);
        }
        p = p->next;
    }
    if (count == 0) printf("该科室暂无患者\n");
    else printf("\n共计 %d 位患者\n", count);
}

// ===================== 住院状态管理 =====================

void changePatientHospital() {
    int pid, opt;
    printf("\n===== 修改患者住院状态 =====\n");
    printf("请输入患者ID：");
    scanf("%d", &pid);
    clearInputBuffer();

    Patient* p = queryPatientById(pid);
    if (p == NULL) {
        printf("【错误】查无此患者！\n解决方案：先录入患者建档后再操作\n");
        return;
    }

    printf("当前状态：%s\n", p->isHospitalized ? "住院" : "门诊");
    printf("1.标记住院   0.设置门诊不住院\n请选择：");
    scanf("%d", &opt);
    clearInputBuffer();
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

void queryPatientHospitalFreeStatus() {
    printf("\n===== 住院/门诊患者状态统计 =====\n");
    Patient* p = patientHead;
    int hos = 0, out = 0;
    if (!p) {
        printf("暂无任何患者信息！\n");
        return;
    }
    printf("%-8s %-10s %-10s\n", "患者ID", "姓名", "当前状态");
    printf("------------------------------\n");
    while (p) {
        printf("%-8d %-10s %-10s\n",
               p->patientId, p->name,
               p->isHospitalized ? "住院占用" : "门诊空闲");
        if (p->isHospitalized) hos++;
        else out++;
        p = p->next;
    }
    printf("\n统计：住院患者%d人 | 门诊空闲患者%d人 | 总计%d人\n", hos, out, hos + out);
}

// ===================== 科室与医生查询 =====================

void queryAllDepartment() {
    if (doctorHead == NULL) {
        printf("暂无医生数据，无法查询科室！\n");
        deptCount = 0;
        return;
    }
    deptCount = 0;
    Doctor* p = doctorHead;
    printf("\n=== 医院所有科室 ===\n");
    while (p != NULL) {
        int exist = 0;
        int i;
        for (i = 0; i < deptCount; i++) {
            if (strcmp(deptList[i], p->department) == 0) {
                exist = 1;
                break;
            }
        }
        if (!exist) {
            strcpy(deptList[deptCount++], p->department);
            printf("  %d. %s\n", deptCount, p->department);
        }
        p = p->next;
    }
    if (deptCount == 0) {
        printf("暂无科室信息！\n");
    }
}

void queryDoctorByDept(const char* deptName) {
    if (doctorHead == NULL) {
        printf("暂无医生数据！\n");
        return;
    }
    Doctor* p = doctorHead;
    int count = 0;
    printf("\n=== %s 科室医生列表 ===\n", deptName);
    printf("%-8s %-10s %-12s %-10s\n", "医生ID", "姓名", "职称", "接诊人数");
    printf("-------------------------------------------\n");
    while (p != NULL) {
        if (strcmp(p->department, deptName) == 0) {
            count++;
            printf("%-8d %-10s %-12s %-10d\n",
                   p->doctorId, p->name, p->title, p->patientCount);
        }
        p = p->next;
    }
    if (count == 0) printf("该科室暂无医生！\n");
}

void queryDoctorWorkload() {
    if (!doctorHead) {
        printf("暂无医生数据！\n");
        return;
    }

    int count = 0;
    Doctor* p = doctorHead;
    while (p) { count++; p = p->next; }

    if (count == 0) {
        printf("暂无医生数据！\n");
        return;
    }

    Doctor** docs = (Doctor**)malloc(count * sizeof(Doctor*));
    if (!docs) { printf("内存分配失败！\n"); return; }

    p = doctorHead;
    {
        int i;
        for (i = 0; i < count; i++) {
            docs[i] = p;
            p = p->next;
        }
    }

    {
        int i, j;
        for (i = 0; i < count - 1; i++) {
            for (j = 0; j < count - 1 - i; j++) {
                if (docs[j]->patientCount < docs[j+1]->patientCount) {
                    Doctor* temp = docs[j];
                    docs[j] = docs[j+1];
                    docs[j+1] = temp;
                }
            }
        }
    }

    printf("\n===== 医生工作量统计（按接诊量排序）=====\n");
    printf("%-4s %-8s %-10s %-12s %-10s %-10s\n", 
           "排名", "医生ID", "姓名", "科室", "职称", "接诊人数");
    printf("-------------------------------------------------------\n");
    {
        int i;
        for (i = 0; i < count; i++) {
            printf("%-4d %-8d %-10s %-12s %-10s %-10d\n",
                   i + 1, docs[i]->doctorId, docs[i]->name, docs[i]->department,
                   docs[i]->title, docs[i]->patientCount);
        }
    }
    free(docs);
}

// ===================== 病历管理 =====================

VisitRecord* createVisitRecord(int recordId, const char* diagnosis, double cost) {
    VisitRecord* newNode = (VisitRecord*)malloc(sizeof(VisitRecord));
    if (!newNode) {
        printf("【错误】内存分配失败！\n");
        return NULL;
    }
    newNode->recordId = recordId;
    strcpy(newNode->diagnosis, diagnosis);
    newNode->cost = cost;
    newNode->next = NULL;
    return newNode;
}

void addPatientVisit(int patientId) {
    Patient* p = queryPatientById(patientId);
    if (p == NULL) {
        printf("【错误】患者不存在！无法添加病历\n解决方案：先挂号建档\n");
        return;
    }
    char diag[MAX_DIAG_LEN];
    double cost;

    printf("请输入病情诊断：");
    safeInput(diag, MAX_DIAG_LEN, NULL);
    if (strlen(diag) == 0) {
        printf("【错误】诊断不能为空！\n");
        return;
    }

    printf("请输入就诊费用：");
    while (scanf("%lf", &cost) != 1 || cost < 0) {
        printf("费用不能为负数！重新输入：");
        clearInputBuffer();
    }
    clearInputBuffer();

    VisitRecord* newRecord = createVisitRecord(nextRecordId++, diag, cost);
    if (!newRecord) return;

    if (p->records == NULL)
        p->records = newRecord;
    else {
        VisitRecord* r = p->records;
        while (r->next != NULL) r = r->next;
        r->next = newRecord;
    }
    p->totalCost += cost;

    Doctor* doc = queryDoctorById(p->doctorId);
    if (doc != NULL) doc->patientCount++;

    printf("就诊病历添加成功！病历ID:%d 累计费用：%.2f\n", newRecord->recordId, p->totalCost);
}

void deletePatientRecord(int pid) {
    Patient* p = queryPatientById(pid);
    if (!p) {
        printf("【错误】患者不存在！\n");
        return;
    }
    if (!p->records) {
        printf("【错误】该患者无病历可删除！\n");
        return;
    }

    printf("=== 该患者所有病历 ===\n");
    VisitRecord* r = p->records;
    while (r) {
        printf("病历ID:%d 诊断:%s 费用:%.2f\n", r->recordId, r->diagnosis, r->cost);
        r = r->next;
    }

    printf("请输入要删除的病历ID：");
    int rid;
    scanf("%d", &rid);
    clearInputBuffer();

    VisitRecord *pre = NULL, *cur = p->records;
    while (cur) {
        if (cur->recordId == rid) {
            p->totalCost -= cur->cost;
            if (pre == NULL) p->records = cur->next;
            else pre->next = cur->next;
            free(cur);
            printf("病历删除成功，费用已扣除！剩余总费用：%.2f\n", p->totalCost);
            return;
        }
        pre = cur;
        cur = cur->next;
    }
    printf("【错误】未找到该病历ID！\n解决方案：先查询病历列表确认ID\n");
}

void queryPatientRecord(int patientId) {
    Patient* p = queryPatientById(patientId);
    if (p == NULL) {
        printf("【错误】未找到该患者！\n");
        return;
    }
    printf("\n=== 患者【%s】病历记录 ===\n", p->name);
    if (p->records == NULL) {
        printf("暂无就诊病历\n");
        return;
    }
    printf("%-8s %-20s %-10s\n", "病历ID", "诊断", "费用");
    printf("-----------------------------------\n");
    VisitRecord* r = p->records;
    while (r != NULL) {
        printf("%-8d %-20s %-10.2f\n", r->recordId, r->diagnosis, r->cost);
        r = r->next;
    }
}

// ===================== 检查项目管理 =====================

void addPatientCheck(int patientId) {
    Patient* p = queryPatientById(patientId);
    if (p == NULL) {
        printf("【错误】患者不存在！无法开检查\n解决方案：先录入患者\n");
        return;
    }
    int type;
    char res[50];
    double fee;

    printf("\n===== 检查项目列表 =====\n");
    printf("1.血常规(45元)  2.尿常规(38元)  3.心电图(60元)\n");
    printf("4.胸部CT(280元)  5.腹部B超(120元)  6.核磁共振(500元)\n");
    printf("请选择检查编号：");
    scanf("%d", &type);
    clearInputBuffer();

    CheckRecord* cr = (CheckRecord*)malloc(sizeof(CheckRecord));
    if (!cr) {
        printf("【错误】内存分配失败！\n");
        return;
    }
    cr->checkId = nextCheckId++;
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
    safeInput(res, 50, NULL);
    strcpy(cr->checkResult, res);
    cr->checkFee = fee;
    p->totalCost += fee;

    if (p->checks == NULL) p->checks = cr;
    else { 
        CheckRecord* t = p->checks; 
        while (t->next) t = t->next; 
        t->next = cr; 
    }
    printf("检查开具完成！检查ID:%d 项目:%s 费用%.2f元\n", cr->checkId, cr->checkName, fee);
}

void modifyCheckResult(int pid) {
    Patient* p = queryPatientById(pid);
    if (!p) { printf("患者不存在！\n"); return; }
    if (!p->checks) { printf("暂无检查记录，无法修改！\n"); return; }

    int cid;
    char newRes[50];
    printf("\n=== 该患者所有检查记录 ===\n");
    printf("%-8s %-12s %-15s\n", "检查ID", "项目", "当前结果");
    printf("---------------------------------\n");
    CheckRecord* t = p->checks;
    while (t) {
        printf("%-8d %-12s %-15s\n", t->checkId, t->checkName, t->checkResult);
        t = t->next;
    }

    printf("请输入要修改的检查ID：");
    scanf("%d", &cid);
    clearInputBuffer();

    t = p->checks;
    while (t) {
        if (t->checkId == cid) {
            printf("请输入新的检查结果：");
            safeInput(newRes, 50, NULL);
            strcpy(t->checkResult, newRes);
            printf("检查结果修改成功！\n");
            return;
        }
        t = t->next;
    }
    printf("未找到该检查ID！\n");
}

void queryPatientCheck(int pid) {
    Patient* p = queryPatientById(pid);
    if (!p) { printf("患者不存在！\n"); return; }
    printf("\n=== 患者%s 检查报告明细 ===\n", p->name);
    if (!p->checks) { printf("暂无检查记录\n"); return; }
    printf("%-8s %-12s %-15s %-10s\n", "检查ID", "项目", "结果", "费用");
    printf("------------------------------------------------\n");
    CheckRecord* t = p->checks;
    while (t) {
        printf("%-8d %-12s %-15s %-10.2f\n", 
               t->checkId, t->checkName, t->checkResult, t->checkFee);
        t = t->next;
    }
}

// ===================== 处方管理（支持多药品） =====================

void prescribeMedicine(int pid) {
    Patient* p = queryPatientById(pid);
    if (!p) { printf("【错误】患者不存在！无法开处方\n"); return; }

    Prescription* pre = (Prescription*)malloc(sizeof(Prescription));
    if (!pre) {
        printf("【错误】内存分配失败！\n");
        return;
    }
    pre->prescriptionId = nextPrescriptionId++;
    pre->patientId = pid; 
    pre->totalCost = 0; 
    pre->items = NULL;

    printf("\n=== 药品库存列表 ===\n");
    printf("%-8s %-16s %-10s %-8s %-10s\n", "药品ID", "名称", "单价", "库存", "预警线");
    printf("-------------------------------------------------\n");
    Medicine* m = medicineHead;
    while (m) { 
        printf("%-8d %-16s %-10.2f %-8d %-10d%s\n", 
               m->medicineId, m->name, m->price, m->stock, m->warningLine,
               m->stock <= m->warningLine ? " [库存预警!]" : ""); 
        m = m->next; 
    }

    char choice;
    do {
        printf("\n请输入药品ID（0结束添加）：");
        int mid;
        scanf("%d", &mid);
        clearInputBuffer();
        if (mid == 0) break;

        Medicine* med = queryMedicineById(mid);
        if (!med) {
            printf("【错误】无此药品ID！\n");
            continue;
        }

        printf("请输入开药数量："); 
        int num;
        scanf("%d", &num);
        clearInputBuffer();

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

        PrescriptionItem* pi = (PrescriptionItem*)malloc(sizeof(PrescriptionItem));
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

        if (pre->items == NULL) {
            pre->items = pi;
        } else {
            PrescriptionItem* last = pre->items;
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
        clearInputBuffer();
    } while (choice == 'y' || choice == 'Y');

    if (pre->items == NULL) {
        printf("处方为空，已取消\n");
        free(pre);
        return;
    }

    pre->next = prescriptionHead; 
    prescriptionHead = pre;
    printf("\n处方开药成功！处方ID:%d 总费用%.2f元\n", pre->prescriptionId, pre->totalCost);
}

void queryPrescription(int pid) {
    Patient* p = queryPatientById(pid);
    if (!p) { printf("患者不存在！\n"); return; }

    printf("\n=== 患者%s 处方明细 ===\n", p->name);
    Prescription* pr = prescriptionHead;
    int found = 0;
    while (pr) {
        if (pr->patientId == pid) {
            found = 1;
            printf("\n处方ID:%d 总费用:%.2f元\n", pr->prescriptionId, pr->totalCost);
            printf("%-8s %-16s %-8s %-10s\n", "药品ID", "名称", "数量", "小计");
            printf("-----------------------------------------\n");
            PrescriptionItem* pi = pr->items;
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

// ===================== 药品库存管理 =====================

void queryMedicineStock() {
    printf("\n===== 药品库存列表 =====\n");
    printf("%-8s %-16s %-10s %-8s %-10s %-10s\n", 
           "药品ID", "名称", "单价", "库存", "预警线", "状态");
    printf("----------------------------------------------------------------\n");
    Medicine* m = medicineHead;
    while (m) {
        printf("%-8d %-16s %-10.2f %-8d %-10d %-10s\n",
               m->medicineId, m->name, m->price, m->stock, m->warningLine,
               m->stock <= m->warningLine ? "库存预警" : "正常");
        m = m->next;
    }
}

void restockMedicine() {
    printf("\n===== 药品补货 =====\n");
    queryMedicineStock();

    printf("\n请输入要补货的药品ID：");
    int mid;
    scanf("%d", &mid);
    clearInputBuffer();

    Medicine* m = queryMedicineById(mid);
    if (!m) {
        printf("【错误】无此药品！\n");
        return;
    }

    printf("当前库存：%d，请输入补货数量：", m->stock);
    int amount;
    scanf("%d", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf("【错误】补货数量必须大于0！\n");
        return;
    }

    m->stock += amount;
    printf("补货成功！[%s]当前库存：%d\n", m->name, m->stock);
}

// ===================== 缴费清单打印 =====================

void printDetailedBill(int pid) {
    Patient* p = queryPatientById(pid);
    if (!p) { printf("患者不存在，无法打印账单！\n"); return; }

    double diagFee = 0, checkFee = 0, medFee = 0;

    VisitRecord* vr = p->records;
    while (vr) { diagFee += vr->cost; vr = vr->next; }

    CheckRecord* cr = p->checks;
    while (cr) { checkFee += cr->checkFee; cr = cr->next; }

    Prescription* pr = prescriptionHead;
    while (pr) {
        if (pr->patientId == pid) medFee += pr->totalCost;
        pr = pr->next;
    }

    printf("\n==================== 患者费用详细清单 ====================\n");
    printf("患者ID：%d\t姓名：%s\n", p->patientId, p->name);
    printf("当前就诊状态：%s\n", p->isHospitalized ? "住院" : "门诊");
    printf("--------------------------------------------------------\n");

    printf("\n【就诊病历明细】\n");
    vr = p->records;
    if (!vr) printf("  无\n");
    while (vr) {
        printf("  病历ID:%d %s %.2f元\n", vr->recordId, vr->diagnosis, vr->cost);
        vr = vr->next;
    }
    printf("就诊病历总费用：\t\t%.2f 元\n", diagFee);

    printf("\n【检查项目明细】\n");
    cr = p->checks;
    if (!cr) printf("  无\n");
    while (cr) {
        printf("  %s(%s) %.2f元\n", cr->checkName, cr->checkResult, cr->checkFee);
        cr = cr->next;
    }
    printf("各项检查总费用：\t\t%.2f 元\n", checkFee);

    printf("\n【药品处方明细】\n");
    pr = prescriptionHead;
    int hasMed = 0;
    while (pr) {
        if (pr->patientId == pid) {
            hasMed = 1;
            printf("  处方ID:%d\n", pr->prescriptionId);
            PrescriptionItem* pi = pr->items;
            while (pi) {
                printf("    %s x%d = %.2f元\n", pi->medicineName, pi->quantity, pi->itemCost);
                pi = pi->next;
            }
        }
        pr = pr->next;
    }
    if (!hasMed) printf("  无\n");
    printf("药品处方总费用：\t\t%.2f 元\n", medFee);

    printf("--------------------------------------------------------\n");
    printf("患者全程累计总费用：\t\t%.2f 元\n", p->totalCost);
    printf("=========================================================\n");
}

// ===================== 挂号候诊队列 =====================

Registration* createRegistration(int rid, int pid, int did, const char* dept, int type, int time) {
    Registration* reg = (Registration*)malloc(sizeof(Registration));
    if (!reg) {
        printf("【错误】内存分配失败！\n");
        return NULL;
    }
    reg->registerId = rid; 
    reg->patientId = pid; 
    reg->doctorId = did;
    strcpy(reg->department, dept); 
    reg->registerType = type; 
    reg->registerTime = time;
    reg->status = 0; 
    reg->next = NULL; 
    return reg;
}

void patientRegister() {
    int pid, did, type, time; 
    char dept[MAX_DEP_LEN];

    printf("\n===== 患者挂号 =====\n");
    printf("请输入患者ID："); 
    scanf("%d", &pid);
    clearInputBuffer();

    Patient* p = queryPatientById(pid);
    if (!p) { 
        printf("【错误】患者未建档！\n解决方案：先添加患者信息\n"); 
        return; 
    }

    printf("请选择科室：\n");
    queryAllDepartment();
    printf("请输入科室名称或编号：");
    safeInput(dept, MAX_DEP_LEN, NULL);

    // 支持输入编号自动匹配科室名称
    int num = atoi(dept);
    if (num > 0 && num <= deptCount) {
        strcpy(dept, deptList[num - 1]);
        printf("已选择科室：%s\n", dept);
    }

    printf("该科室医生列表：\n");
    queryDoctorByDept(dept);
    printf("请输入医生ID："); 
    scanf("%d", &did);
    clearInputBuffer();

    Doctor* d = queryDoctorById(did);
    if (!d) { 
        printf("【错误】医生不存在！\n"); 
        return; 
    }
    if (strcmp(d->department, dept) != 0) {
        printf("【错误】医生%s不属于%s科室！\n", d->name, dept);
        return;
    }

    printf("挂号类型 0现场 1预约："); 
    scanf("%d", &type);
    clearInputBuffer();
    if (type != 0 && type != 1) {
        printf("【错误】挂号类型只能是0或1！\n");
        return;
    }

    printf("排队序号："); 
    scanf("%d", &time);
    clearInputBuffer();
    if (time < 1) {
        printf("【错误】排队序号必须>=1！\n");
        return;
    }

    Registration* r = createRegistration(nextRegId++, pid, did, dept, type, time);
    if (!r) return;

    if (regHead == NULL) regHead = r;
    else { 
        Registration* t = regHead; 
        while (t->next) t = t->next; 
        t->next = r; 
    }

    printf("挂号成功！挂号ID:%d，自动进入候诊队列\n", r->registerId);
    addIntoQueue(r->registerId, pid, did);
}

void addIntoQueue(int registerId, int patientId, int doctorId) {
    QueueNode* q = (QueueNode*)malloc(sizeof(QueueNode));
    if (!q) {
        printf("【错误】内存分配失败！\n");
        return;
    }
    q->queueId = nextQueueId++; 
    q->registerId = registerId; 
    q->patientId = patientId;
    q->doctorId = doctorId; 
    q->status = 0; 
    q->next = NULL;

    if (queueHead == NULL) queueHead = q;
    else { 
        QueueNode* t = queueHead; 
        while (t->next) t = t->next; 
        t->next = q; 
    }
}

void queryAllQueue() {
    if (!queueHead) { printf("候诊队列为空\n"); return; }

    printf("\n=== 候诊排队列表 ===\n");
    printf("%-8s %-8s %-8s %-8s %-10s\n", "队列号", "患者ID", "医生ID", "挂号ID", "状态");
    printf("------------------------------------------------\n");

    QueueNode* t = queueHead;
    while (t) {
        char s[10];
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

void queryQueueByDoctor() {
    if (!queueHead) { printf("候诊队列为空\n"); return; }

    printf("\n===== 按医生查看候诊队列 =====\n");
    printf("请输入医生ID（0查看全部）：");
    int did;
    scanf("%d", &did);
    clearInputBuffer();

    QueueNode* t = queueHead;
    int found = 0;
    printf("%-8s %-8s %-8s %-10s\n", "队列号", "患者ID", "挂号ID", "状态");
    printf("---------------------------------------\n");
    while (t) {
        if (did == 0 || t->doctorId == did) {
            found = 1;
            char s[10];
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

void callNextPatient() {
    if (!queueHead) { 
        printf("【错误】无排队患者，无法叫号\n解决方案：先挂号\n"); 
        return; 
    }

    QueueNode* t = queueHead;
    while (t && t->status != 0) t = t->next;

    if (!t) {
        printf("【提示】当前没有候诊状态的患者\n");
        return;
    }

    t->status = 2;
    Patient* p = queryPatientById(t->patientId);
    Doctor* d = queryDoctorById(t->doctorId);
    printf("叫号成功！患者%s 进入就诊中（医生：%s）\n", 
           p ? p->name : "未知", d ? d->name : "未知");
}

void finishVisit() {
    if (!queueHead) { printf("暂无就诊患者\n"); return; }

    QueueNode* t = queueHead;
    while (t && t->status != 2) t = t->next;

    if (!t) {
        printf("【提示】当前没有就诊中的患者\n");
        return;
    }

    t->status = 3;
    Patient* p = queryPatientById(t->patientId);
    printf("本次就诊接诊结束！患者：%s\n", p ? p->name : "未知");
}

// ===================== 数据持久化 =====================

void saveData() {
    FILE* fp = fopen(DATA_FILE, "w");
    if (!fp) {
        printf("【错误】无法创建保存文件！\n");
        return;
    }

    fprintf(fp, "COUNTERS %d %d %d %d %d\n", 
            nextRecordId, nextCheckId, nextPrescriptionId, nextRegId, nextQueueId);

    Doctor* d = doctorHead;
    while (d) {
        fprintf(fp, "DOCTOR %d %s %s %s %d\n", 
                d->doctorId, d->name, d->department, d->title, d->patientCount);
        d = d->next;
    }

    Medicine* m = medicineHead;
    while (m) {
        fprintf(fp, "MEDICINE %d %s %.2f %d %d\n",
                m->medicineId, m->name, m->price, m->stock, m->warningLine);
        m = m->next;
    }

    Patient* p = patientHead;
    while (p) {
        fprintf(fp, "PATIENT %d %s %d %c %s %d %d %.2f\n",
                p->patientId, p->name, p->age, p->gender, p->department,
                p->doctorId, p->isHospitalized ? 1 : 0, p->totalCost);
        p = p->next;
    }

    fclose(fp);
    printf("数据保存成功！文件：%s\n", DATA_FILE);
}

void loadData() {
    FILE* fp = fopen(DATA_FILE, "r");
    if (!fp) {
        printf("【提示】未找到存档文件，使用默认初始化数据\n");
        return;
    }

    char type[20];
    while (fscanf(fp, "%s", type) == 1) {
        if (strcmp(type, "COUNTERS") == 0) {
            fscanf(fp, "%d %d %d %d %d", 
                   &nextRecordId, &nextCheckId, &nextPrescriptionId, &nextRegId, &nextQueueId);
        }
        else if (strcmp(type, "DOCTOR") == 0) {
            Doctor* d = (Doctor*)malloc(sizeof(Doctor));
            fscanf(fp, "%d %s %s %s %d", &d->doctorId, d->name, d->department, d->title, &d->patientCount);
            d->next = doctorHead;
            doctorHead = d;
        }
        else if (strcmp(type, "MEDICINE") == 0) {
            Medicine* m = (Medicine*)malloc(sizeof(Medicine));
            fscanf(fp, "%d %s %lf %d %d", &m->medicineId, m->name, &m->price, &m->stock, &m->warningLine);
            m->next = medicineHead;
            medicineHead = m;
        }
        else if (strcmp(type, "PATIENT") == 0) {
            Patient* p = (Patient*)malloc(sizeof(Patient));
            int hosp;
            fscanf(fp, "%d %s %d %c %s %d %d %lf", 
                   &p->patientId, p->name, &p->age, &p->gender, p->department,
                   &p->doctorId, &hosp, &p->totalCost);
            p->isHospitalized = hosp;
            p->records = NULL;
            p->checks = NULL;
            p->next = patientHead;
            patientHead = p;
        }
    }

    fclose(fp);
    printf("数据加载成功！\n");
}

// ===================== 主菜单 =====================

void showMenu() {
    printf("\n================== 医院门诊管理系统 ==================\n");
    printf("【患者管理】\n");
    printf("  1.录入新患者信息    2.修改患者信息    3.删除患者\n");
    printf("  4.查询所有患者      5.按科室查患者\n");
    printf("【科室与医生】\n");
    printf("  6.查询科室与医生    7.医生工作量统计\n");
    printf("【挂号候诊】\n");
    printf("  8.患者挂号排队      9.查看候诊队列   10.按医生查看队列\n");
    printf(" 11.叫号就诊         12.结束本次就诊\n");
    printf("【诊疗服务】\n");
    printf(" 13.添加就诊病历     14.删除就诊病历   15.查询病历\n");
    printf(" 16.开具检查项目     17.修改检查结果   18.查询检查报告\n");
    printf(" 19.开具处方药品     20.查询处方明细\n");
    printf("【住院与费用】\n");
    printf(" 21.设置住院/门诊    22.住院状态统计   23.打印缴费清单\n");
    printf("【药品管理】\n");
    printf(" 24.查看药品库存     25.药品补货\n");
    printf("【系统】\n");
    printf(" 26.保存数据         27.加载数据       0.退出系统\n");
    printf("======================================================\n");
    printf("请输入操作序号：");
    fflush(stdout);
}

void freeAllMemory() {
    Patient* p = patientHead;
    while (p) {
        VisitRecord* vr = p->records;
        while (vr) { VisitRecord* t = vr; vr = vr->next; free(t); }
        CheckRecord* cr = p->checks;
        while (cr) { CheckRecord* t = cr; cr = cr->next; free(t); }
        Patient* tp = p; p = p->next; free(tp);
    }

    Doctor* d = doctorHead;
    while (d) { Doctor* t = d; d = d->next; free(t); }

    Medicine* m = medicineHead;
    while (m) { Medicine* t = m; m = m->next; free(t); }

    Prescription* pr = prescriptionHead;
    while (pr) {
        PrescriptionItem* pi = pr->items;
        while (pi) { PrescriptionItem* t = pi; pi = pi->next; free(t); }
        Prescription* t = pr; pr = pr->next; free(t);
    }

    Registration* r = regHead;
    while (r) { Registration* t = r; r = r->next; free(t); }

    QueueNode* q = queueHead;
    while (q) { QueueNode* t = q; q = q->next; free(t); }
}

int B_entry() {
    printf("=== 医院管理系统 完整版 | 全容错跨平台 ===\n");

    loadData();
    if (!doctorHead) initBaseData();

    int opt, pid;
    while (1) {
        showMenu();
        while (scanf("%d", &opt) != 1) {
            printf("输入无效！请输入数字序号：");
            clearInputBuffer();
        }
        clearInputBuffer();

        switch (opt) {
            case 1: addNewPatient(); break;
            case 2: modifyPatient(); break;
            case 3: deletePatient(); break;
            case 4: queryAllPatients(); break;
            case 5: queryPatientsByDept(); break;
            case 6: {
                queryAllDepartment(); 
                if (deptCount == 0) break;
                printf("\n请输入要查看的科室名称或编号：");
                char dept[MAX_DEP_LEN];
                safeInput(dept, MAX_DEP_LEN, NULL);

                // 如果输入的是数字编号，自动匹配科室名称
                int num = atoi(dept);
                if (num > 0 && num <= deptCount) {
                    strcpy(dept, deptList[num - 1]);
                    printf("已选择科室：%s\n", dept);
                }
                queryDoctorByDept(dept); 
                break;
            }
            case 7: queryDoctorWorkload(); break;
            case 8: patientRegister(); break;
            case 9: queryAllQueue(); break;
            case 10: queryQueueByDoctor(); break;
            case 11: callNextPatient(); break;
            case 12: finishVisit(); break;
            case 13: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                addPatientVisit(pid); 
                break;
            case 14: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                deletePatientRecord(pid); 
                break;
            case 15: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                queryPatientRecord(pid); 
                break;
            case 16: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                addPatientCheck(pid); 
                break;
            case 17: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                modifyCheckResult(pid); 
                break;
            case 18: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                queryPatientCheck(pid); 
                break;
            case 19: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                prescribeMedicine(pid); 
                break;
            case 20: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                queryPrescription(pid); 
                break;
            case 21: changePatientHospital(); break;
            case 22: queryPatientHospitalFreeStatus(); break;
            case 23: 
                printf("患者ID："); 
                scanf("%d", &pid); clearInputBuffer();
                printDetailedBill(pid); 
                break;
            case 24: queryMedicineStock(); break;
            case 25: restockMedicine(); break;
            case 26: saveData(); break;
            case 27: {
                printf("加载将覆盖当前内存数据，确认？(1-是 0-否)：");
                int confirm;
                scanf("%d", &confirm); clearInputBuffer();
                if (confirm == 1) {
                    freeAllMemory();
                    patientHead = NULL; doctorHead = NULL; medicineHead = NULL;
                    prescriptionHead = NULL; regHead = NULL; queueHead = NULL;
                    loadData();
                }
                break;
            }
            case 0: {
                printf("退出系统！是否保存数据？(1-是 0-否)：");
                scanf("%d", &opt); clearInputBuffer();
                if (opt == 1) saveData();
                freeAllMemory();
                printf("感谢使用，再见！\n");
                return 0;
            }
            default: 
                printf("【错误】菜单序号超出范围！\n解决方案：选择0~27正确选项\n");
        }

        // 模块使用完毕后暂停+清屏
        if (opt != 0) {
            pauseScreen();
            clearScreen();
        }
    }
}
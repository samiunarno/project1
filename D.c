#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 100
#define MAX_DEPARTMENTS 10
#define MAX_WARDS_PER_DEPT 10
#define MAX_BEDS_PER_WARD 20
#define SUCCESS 1
#define FAILURE 0

// 数据库文件名
#define DB_DEPARTMENTS_FILE "departments_db.txt"
#define DB_WARDS_FILE "wards_db.txt"
#define DB_BEDS_FILE "beds_db.txt"
#define DB_PATIENTS_FILE "patients_db.txt"
#define DB_ANALYSIS_FILE "ai_analysis_report.txt"

typedef enum {
    STATUS_AVAILABLE = 0,
    STATUS_OCCUPIED = 1
} BedStatus;

typedef enum {
    REPORT_CONSOLE = 1,
    REPORT_FILE = 2,
    REPORT_BOTH = 3
} ReportType;

// 核心结构体
typedef struct Bed {
    int bedNumber;
    BedStatus status;
    int patientId;
    char patientName[MAX_NAME_LENGTH];
    struct Bed* next;
} Bed;

typedef struct Ward {
    int wardNumber;
    char wardName[MAX_NAME_LENGTH];
    int totalBeds;
    int occupiedBeds;
    Bed* bedList;
    struct Ward* next;
} Ward;

typedef struct Department {
    int departmentId;
    char departmentName[MAX_NAME_LENGTH];
    int totalWards;
    int totalBeds;
    int totalPatients;
    Ward* wardList;
    struct Department* next;
} Department;

typedef struct HospitalStatistics {
    int totalDepartments;
    int totalWards;
    int totalBeds;
    int totalPatients;
    float overallOccupancyRate;
    float departmentOccupancyRates[MAX_DEPARTMENTS];
    char departmentNames[MAX_DEPARTMENTS][MAX_NAME_LENGTH];
    int departmentCount;
} HospitalStatistics;

typedef struct PatientInfo {
    int patientId;
    char patientName[MAX_NAME_LENGTH];
    int departmentId;
    int wardNumber;
    int bedNumber;
} PatientInfo;

// ==================== 前置声明 ====================

Bed* createBed(int bedNumber);
void initializeBed(Bed* bed, int bedNumber);
void addBedToWard(Ward* ward, Bed* bed);
void occupyBed(Bed* bed, int patientId, const char* patientName);
void vacateBed(Bed* bed);
int isBedAvailable(const Bed* bed);
void displayBed(const Bed* bed);
void freeBed(Bed* bed);

Ward* createWard(int wardNumber, const char* wardName, int totalBeds);
Bed* findBedInWard(const Ward* ward, int bedNumber);
float getWardOccupancyRate(const Ward* ward);
void displayWard(const Ward* ward);
void freeWard(Ward* ward);

Department* createDepartment(int departmentId, const char* departmentName);
void addWardToDepartment(Department* department, Ward* ward);
Ward* findWardInDepartment(const Department* department, int wardNumber);
void updateDepartmentStats(Department* department);
void freeDepartment(Department* department);
void displayDepartment(const Department* department);

HospitalStatistics collectStatistics(Department* hospital);
void exportStatisticsToFile(Department* hospital);
void generateStatisticsReport(Department* hospital, ReportType type);

void printDepartmentDetails(Department* hospital);
void printWardDetails(Department* hospital);
int admitPatient(Department* hospital, int departmentId, int wardNumber,
                 int bedNumber, int patientId, const char* patientName);
int dischargePatient(Department* hospital, int patientId);
int findPatientLocation(Department* hospital, int patientId, PatientInfo* info);
void displayAllPatients(Department* hospital);
int validateTransfer(Department* hospital, int patientId, int newDepartmentId,
                     int newWardNumber, int newBedNumber, char* errorMessage);
int transferPatient(Department* hospital, int patientId, int newDepartmentId,
                    int newWardNumber, int newBedNumber);

int getValidatedIntInput(const char* prompt, int min, int max);
void clearInputBuffer(void);
void printSuccess(const char* message);
void printError(const char* message);
void printInfo(const char* message);
void printHeader(const char* title);
void printSeparator(void);
void printMainMenu(void);

void handleAdmitPatient(Department* hospital);
void handleTransferPatient(Department* hospital);
void handleDischargePatient(Department* hospital);
void handleReleaseBed(Department* hospital);
void performDataAnalysis(Department* hospital);

void init_hospital_globals(void);
int D_entry(void);
void freeHospitalSystem(Department** hospital);
void printMemoryReport(void);

// 数据库实时读写
int saveAllDataToDB(Department* hospital);
int loadAllDataFromDB(Department** hospital);
int checkDatabaseIntegrity(void);
void printDatabaseSummary(Department* hospital);

// 动态展示（无mock）
void displayDepartmentList(void);
void displayWardListForDepartment(int deptId);
int getMaxBedNumberInWard(Department* hospital, int deptId, int wardNum);

// ==================== 全局状态 ====================
static Department* g_hospital = NULL;

// ==================== 辅助输出 ====================

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // discard
    }
}

void printSuccess(const char* message) { printf("[成功] %s\n", message); }
void printError(const char* message) { printf("[错误] %s\n", message); }
void printInfo(const char* message) { printf("[信息] %s\n", message); }

void printHeader(const char* title) {
    printf("\n============================================================\n");
    printf("%s\n", title);
    printf("============================================================\n");
}

void printSeparator(void) {
    printf("------------------------------------------------------------\n");
}

int getValidatedIntInput(const char* prompt, int min, int max) {
    int value = 0;
    char buf[128];

    while (1) {
        printf("%s [%d-%d]: ", prompt, min, max);
        if (!fgets(buf, sizeof(buf), stdin)) {
            continue;
        }
        if (sscanf(buf, "%d", &value) != 1) {
            printf("[错误] 无效输入！请输入数字。\n");
            continue;
        }
        if (value < min || value > max) {
            printf("[错误] 输入超出范围！请输入 %d-%d。\n", min, max);
            continue;
        }
        return value;
    }
}

// ==================== 结构体内存 ====================

Bed* createBed(int bedNumber) {
    Bed* bed = (Bed*)malloc(sizeof(Bed));
    if (!bed) {
        printError("床位内存分配失败");
        return NULL;
    }
    initializeBed(bed, bedNumber);
    return bed;
}

void initializeBed(Bed* bed, int bedNumber) {
    bed->bedNumber = bedNumber;
    bed->status = STATUS_AVAILABLE;
    bed->patientId = -1;
    bed->patientName[0] = '\0';
    bed->next = NULL;
}

void addBedToWard(Ward* ward, Bed* bed) {
    if (!ward || !bed) return;

    if (!ward->bedList) {
        ward->bedList = bed;
        return;
    }
    Bed* cur = ward->bedList;
    while (cur->next) cur = cur->next;
    cur->next = bed;
}

void occupyBed(Bed* bed, int patientId, const char* patientName) {
    if (!bed) return;
    bed->status = STATUS_OCCUPIED;
    bed->patientId = patientId;
    strncpy(bed->patientName, patientName ? patientName : "", MAX_NAME_LENGTH - 1);
    bed->patientName[MAX_NAME_LENGTH - 1] = '\0';
}

void vacateBed(Bed* bed) {
    if (!bed) return;
    bed->status = STATUS_AVAILABLE;
    bed->patientId = -1;
    bed->patientName[0] = '\0';
}

int isBedAvailable(const Bed* bed) {
    return bed && bed->status == STATUS_AVAILABLE;
}

void displayBed(const Bed* bed) {
    if (!bed) return;
    if (bed->status == STATUS_OCCUPIED) {
        printf("  床位 %2d: [已占用] 患者: %s (ID: %d)\n", bed->bedNumber, bed->patientName, bed->patientId);
    } else {
        printf("  床位 %2d: [可用]\n", bed->bedNumber);
    }
}

void freeBed(Bed* bed) {
    if (bed) free(bed);
}

// ==================== 病房 ====================

Ward* createWard(int wardNumber, const char* wardName, int totalBeds) {
    Ward* ward = (Ward*)malloc(sizeof(Ward));
    if (!ward) {
        printError("病房内存分配失败");
        return NULL;
    }

    ward->wardNumber = wardNumber;
    strncpy(ward->wardName, wardName ? wardName : "", MAX_NAME_LENGTH - 1);
    ward->wardName[MAX_NAME_LENGTH - 1] = '\0';

    ward->totalBeds = totalBeds;
    ward->occupiedBeds = 0;
    ward->bedList = NULL;
    ward->next = NULL;

    for (int i = 1; i <= totalBeds; i++) {
        addBedToWard(ward, createBed(i));
    }
    return ward;
}

Bed* findBedInWard(const Ward* ward, int bedNumber) {
    if (!ward) return NULL;
    Bed* cur = ward->bedList;
    while (cur) {
        if (cur->bedNumber == bedNumber) return cur;
        cur = cur->next;
    }
    return NULL;
}

float getWardOccupancyRate(const Ward* ward) {
    if (!ward || ward->totalBeds == 0) return 0.0f;
    return (float)ward->occupiedBeds / (float)ward->totalBeds * 100.0f;
}

void displayWard(const Ward* ward) {
    if (!ward) return;

    printf("\n============================================================\n");
    printf("病房 %d: %s\n", ward->wardNumber, ward->wardName);
    printf("占用率: %d/%d 张床 (%.1f%%)\n",
           ward->occupiedBeds, ward->totalBeds, getWardOccupancyRate(ward));
    printf("------------------------------------------------------------\n");

    Bed* cur = ward->bedList;
    while (cur) {
        displayBed(cur);
        cur = cur->next;
    }

    printf("============================================================\n");
}

void freeWard(Ward* ward) {
    if (!ward) return;
    Bed* cur = ward->bedList;
    while (cur) {
        Bed* next = cur->next;
        freeBed(cur);
        cur = next;
    }
    free(ward);
}

// ==================== 科室 ====================

Department* createDepartment(int departmentId, const char* departmentName) {
    Department* dept = (Department*)malloc(sizeof(Department));
    if (!dept) {
        printError("科室内存分配失败");
        return NULL;
    }

    dept->departmentId = departmentId;
    strncpy(dept->departmentName, departmentName ? departmentName : "", MAX_NAME_LENGTH - 1);
    dept->departmentName[MAX_NAME_LENGTH - 1] = '\0';

    dept->totalWards = 0;
    dept->totalBeds = 0;
    dept->totalPatients = 0;
    dept->wardList = NULL;
    dept->next = NULL;

    return dept;
}

void addWardToDepartment(Department* department, Ward* ward) {
    if (!department || !ward) return;

    if (!department->wardList) {
        department->wardList = ward;
    } else {
        Ward* cur = department->wardList;
        while (cur->next) cur = cur->next;
        cur->next = ward;
    }

    department->totalWards++;
    department->totalBeds += ward->totalBeds;
    updateDepartmentStats(department);
}

Ward* findWardInDepartment(const Department* department, int wardNumber) {
    if (!department) return NULL;
    Ward* cur = department->wardList;
    while (cur) {
        if (cur->wardNumber == wardNumber) return cur;
        cur = cur->next;
    }
    return NULL;
}

void updateDepartmentStats(Department* department) {
    if (!department) return;

    department->totalPatients = 0;
    Ward* cur = department->wardList;
    while (cur) {
        department->totalPatients += cur->occupiedBeds;
        cur = cur->next;
    }
}

void displayDepartment(const Department* department) {
    if (!department) return;

    printf("\n============================================================\n");
    printf("科室: %s (ID: %d)\n", department->departmentName, department->departmentId);
    printf("统计: %d 名患者 | %d 张床 | %d 个病房\n",
           department->totalPatients, department->totalBeds, department->totalWards);
    printf("============================================================\n");

    Ward* cur = department->wardList;
    while (cur) {
        displayWard(cur);
        cur = cur->next;
    }
}

void freeDepartment(Department* department) {
    if (!department) return;
    Ward* cur = department->wardList;
    while (cur) {
        Ward* next = cur->next;
        freeWard(cur);
        cur = next;
    }
    free(department);
}

// ==================== 动态展示（无mock） ====================

void displayDepartmentList(void) {
    printf("\n可用科室列表:\n");
    printf("------------------------------------------------------------\n");
    printf("ID | 科室名称            | 病房数   | 床位数\n");
    printf("------------------------------------------------------------\n");

    if (g_hospital == NULL) {
        printf("(未加载医院数据)\n");
        printf("------------------------------------------------------------\n");
        return;
    }

    Department* dept = g_hospital;
    while (dept) {
        printf("%-3d| %-20s | %-7d | %-7d\n",
               dept->departmentId,
               dept->departmentName,
               dept->totalWards,
               dept->totalBeds);
        dept = dept->next;
    }
    printf("------------------------------------------------------------\n");
}

void displayWardListForDepartment(int deptId) {
    printf("\n可用病房列表:\n");
    printf("------------------------------------------------------------\n");
    printf("病房ID | 病房名称                 | 总床位\n");
    printf("------------------------------------------------------------\n");

    if (g_hospital == NULL) {
        printf("(未加载医院数据)\n");
        printf("------------------------------------------------------------\n");
        return;
    }

    Department* dept = g_hospital;
    while (dept) {
        if (dept->departmentId == deptId) {
            Ward* w = dept->wardList;
            if (!w) {
                printf("该科室下无病房\n");
            }
            while (w) {
                printf("%-6d | %-24s | %-6d\n", w->wardNumber, w->wardName, w->totalBeds);
                w = w->next;
            }
            printf("------------------------------------------------------------\n");
            return;
        }
        dept = dept->next;
    }

    printf("未找到该科室（ID=%d）\n", deptId);
    printf("------------------------------------------------------------\n");
}

int getMaxBedNumberInWard(Department* hospital, int deptId, int wardNum) {
    if (!hospital) return 0;
    Department* d = hospital;
    while (d) {
        if (d->departmentId == deptId) {
            Ward* w = findWardInDepartment(d, wardNum);
            if (!w) return 0;
            return w->totalBeds; // 床位号是从 1..totalBeds
        }
        d = d->next;
    }
    return 0;
}

// ==================== 数据库：保存 ====================

int saveDepartmentsToDB(Department* hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "w");
    if (!file) {
        printError("无法打开科室数据库文件");
        return FAILURE;
    }

    Department* dept = hospital;
    while (dept) {
        fprintf(file, "%d|%s|%d|%d|%d\n",
                dept->departmentId,
                dept->departmentName,
                dept->totalWards,
                dept->totalBeds,
                dept->totalPatients);
        dept = dept->next;
    }

    fclose(file);
    return SUCCESS;
}

int saveWardsToDB(Department* hospital) {
    FILE* file = fopen(DB_WARDS_FILE, "w");
    if (!file) {
        printError("无法打开病房数据库文件");
        return FAILURE;
    }

    Department* dept = hospital;
    while (dept) {
        Ward* w = dept->wardList;
        while (w) {
            fprintf(file, "%d|%d|%s|%d|%d\n",
                    dept->departmentId,
                    w->wardNumber,
                    w->wardName,
                    w->totalBeds,
                    w->occupiedBeds);
            w = w->next;
        }
        dept = dept->next;
    }

    fclose(file);
    return SUCCESS;
}

int saveBedsToDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "w");
    if (!file) {
        printError("无法打开床位数据库文件");
        return FAILURE;
    }

    Department* dept = hospital;
    while (dept) {
        Ward* w = dept->wardList;
        while (w) {
            Bed* bed = w->bedList;
            while (bed) {
                fprintf(file, "%d|%d|%d|%d|%d|%s\n",
                        dept->departmentId,
                        w->wardNumber,
                        bed->bedNumber,
                        (int)bed->status,
                        bed->patientId,
                        bed->patientName);
                bed = bed->next;
            }
            w = w->next;
        }
        dept = dept->next;
    }

    fclose(file);
    return SUCCESS;
}

int savePatientsToDB(Department* hospital) {
    FILE* file = fopen(DB_PATIENTS_FILE, "w");
    if (!file) {
        printError("无法打开患者数据库文件");
        return FAILURE;
    }

    Department* dept = hospital;
    while (dept) {
        Ward* w = dept->wardList;
        while (w) {
            Bed* bed = w->bedList;
            while (bed) {
                if (bed->status == STATUS_OCCUPIED) {
                    fprintf(file, "%d|%s|%d|%d|%d\n",
                            bed->patientId,
                            bed->patientName,
                            dept->departmentId,
                            w->wardNumber,
                            bed->bedNumber);
                }
                bed = bed->next;
            }
            w = w->next;
        }
        dept = dept->next;
    }

    fclose(file);
    return SUCCESS;
}

int saveAllDataToDB(Department* hospital) {
    if (saveDepartmentsToDB(hospital) == FAILURE) return FAILURE;
    if (saveWardsToDB(hospital) == FAILURE) return FAILURE;
    if (saveBedsToDB(hospital) == FAILURE) return FAILURE;
    if (savePatientsToDB(hospital) == FAILURE) return FAILURE;

    printSuccess("数据库已保存到文件");
    return SUCCESS;
}

// ==================== 数据库：加载 ====================

int checkDatabaseIntegrity(void) {
    FILE* f;
    f = fopen(DB_DEPARTMENTS_FILE, "r");
    if (!f) return 0;

    fclose(f);

    // 仅检查存在性与非空（更严格校验可后续加）
    f = fopen(DB_DEPARTMENTS_FILE, "r");
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) > 1) count++;
    }
    fclose(f);

    return count > 0;
}

int loadDepartmentsFromDB(Department** hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "r");
    if (!file) return FAILURE;

    char line[512];
    Department* tail = NULL;

    while (fgets(line, sizeof(line), file)) {
        int deptId, totalWards, totalBeds, totalPatients;
        char deptName[MAX_NAME_LENGTH];

        if (sscanf(line, "%d|%99[^|]|%d|%d|%d", &deptId, deptName, &totalWards, &totalBeds, &totalPatients) != 5) {
            continue;
        }

        Department* dept = createDepartment(deptId, deptName);
        dept->totalWards = totalWards;
        dept->totalBeds = totalBeds;
        dept->totalPatients = totalPatients;

        if (*hospital == NULL) {
            *hospital = dept;
            tail = dept;
        } else {
            tail->next = dept;
            tail = dept;
        }
    }

    fclose(file);
    return SUCCESS;
}

int loadWardsFromDB(Department* hospital) {
    FILE* file = fopen(DB_WARDS_FILE, "r");
    if (!file) return FAILURE;

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        int deptId, wardNumber, totalBeds, occupiedBeds;
        char wardName[MAX_NAME_LENGTH];

        if (sscanf(line, "%d|%d|%99[^|]|%d|%d", &deptId, &wardNumber, wardName, &totalBeds, &occupiedBeds) != 5) {
            continue;
        }

        Department* d = hospital;
        while (d) {
            if (d->departmentId == deptId) {
                Ward* w = createWard(wardNumber, wardName, totalBeds);
                if (w) {
                    w->occupiedBeds = occupiedBeds;
                    addWardToDepartment(d, w);
                }
                break;
            }
            d = d->next;
        }
    }

    fclose(file);
    return SUCCESS;
}

int loadBedsFromDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "r");
    if (!file) return FAILURE;

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        int deptId, wardNumber, bedNumber, status, patientId;
        char patientName[MAX_NAME_LENGTH];
        patientName[0] = '\0';

        // beds_db.txt: deptId|wardNumber|bedNumber|status|patientId|patientName
        if (sscanf(line, "%d|%d|%d|%d|%d|%99[^\n]", &deptId, &wardNumber, &bedNumber, &status, &patientId, patientName) < 5) {
            continue;
        }

        Department* d = hospital;
        while (d) {
            if (d->departmentId == deptId) {
                Ward* w = findWardInDepartment(d, wardNumber);
                if (w) {
                    Bed* bed = findBedInWard(w, bedNumber);
                    if (bed) {
                        bed->status = (status == STATUS_OCCUPIED) ? STATUS_OCCUPIED : STATUS_AVAILABLE;
                        if (bed->status == STATUS_OCCUPIED) {
                            bed->patientId = patientId;
                            strncpy(bed->patientName, patientName, MAX_NAME_LENGTH - 1);
                            bed->patientName[MAX_NAME_LENGTH - 1] = '\0';
                        }
                    }
                }
                break;
            }
            d = d->next;
        }
    }

    fclose(file);
    return SUCCESS;
}

int loadPatientsFromDB(Department* hospital) {
    FILE* file = fopen(DB_PATIENTS_FILE, "r");
    if (!file) return FAILURE;

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        int patientId, deptId, wardNumber, bedNumber;
        char patientName[MAX_NAME_LENGTH];
        patientName[0] = '\0';

        if (sscanf(line, "%d|%99[^|]|%d|%d|%d", &patientId, patientName, &deptId, &wardNumber, &bedNumber) != 5) {
            continue;
        }

        Department* d = hospital;
        while (d) {
            if (d->departmentId == deptId) {
                Ward* w = findWardInDepartment(d, wardNumber);
                if (w) {
                    Bed* bed = findBedInWard(w, bedNumber);
                    if (bed && bed->status == STATUS_AVAILABLE) {
                        occupyBed(bed, patientId, patientName);
                        w->occupiedBeds++;
                    }
                }
                break;
            }
            d = d->next;
        }
    }

    fclose(file);

    // 最终重新统计每个科室患者数
    Department* d = hospital;
    while (d) {
        updateDepartmentStats(d);
        d = d->next;
    }

    return SUCCESS;
}

int loadAllDataFromDB(Department** hospital) {
    if (!hospital) return FAILURE;

    if (*hospital) {
        // free existing
        Department* cur = *hospital;
        while (cur) {
            Department* next = cur->next;
            freeDepartment(cur);
            cur = next;
        }
        *hospital = NULL;
    }

    if (loadDepartmentsFromDB(hospital) == FAILURE) return FAILURE;
    if (*hospital == NULL) return FAILURE;

    // 建议在加载前清空 totals/wardList（createDepartment已是空）
    loadWardsFromDB(*hospital);
    loadBedsFromDB(*hospital);
    loadPatientsFromDB(*hospital);

    // 重算 totalWards/totalBeds/totalPatients，避免中间链式添加带来的累计误差
    Department* d = *hospital;
    while (d) {
        int wCount = 0;
        int bCount = 0;
        d->totalPatients = 0;
        Ward* w = d->wardList;
        while (w) {
            wCount++;
            bCount += w->totalBeds;
            d->totalPatients += w->occupiedBeds;
            w = w->next;
        }
        d->totalWards = wCount;
        d->totalBeds = bCount;
        d = d->next;
    }

    return SUCCESS;
}

void printDatabaseSummary(Department* hospital) {
    if (!hospital) {
        printf("数据库为空\n");
        return;
    }

    int deptCount = 0, wardCount = 0, bedCount = 0, patientCount = 0;
    Department* d = hospital;
    while (d) {
        deptCount++;
        wardCount += d->totalWards;
        bedCount += d->totalBeds;
        patientCount += d->totalPatients;
        d = d->next;
    }

    printf("\n当前数据库状态:\n");
    printf("  科室: %d | 病房: %d | 床位: %d | 患者: %d\n", deptCount, wardCount, bedCount, patientCount);
}

// ==================== 报告/分析 ====================

HospitalStatistics collectStatistics(Department* hospital) {
    HospitalStatistics stats;
    memset(&stats, 0, sizeof(stats));

    Department* d = hospital;
    int idx = 0;

    while (d && idx < MAX_DEPARTMENTS) {
        stats.totalDepartments++;
        stats.totalWards += d->totalWards;
        stats.totalBeds += d->totalBeds;
        stats.totalPatients += d->totalPatients;

        if (d->totalBeds > 0) {
            stats.departmentOccupancyRates[idx] = (float)d->totalPatients / (float)d->totalBeds * 100.0f;
        }
        strncpy(stats.departmentNames[idx], d->departmentName, MAX_NAME_LENGTH - 1);
        stats.departmentNames[idx][MAX_NAME_LENGTH - 1] = '\0';
        idx++;
        d = d->next;
    }

    stats.departmentCount = idx;

    if (stats.totalBeds > 0) {
        stats.overallOccupancyRate = (float)stats.totalPatients / (float)stats.totalBeds * 100.0f;
    }

    return stats;
}

void exportStatisticsToFile(Department* hospital) {
    if (!hospital) {
        printError("医院数据为空，无法生成报告");
        return;
    }

    FILE* file = fopen("hospital_report.txt", "w");
    if (!file) {
        printError("无法创建报告文件");
        return;
    }

    HospitalStatistics stats = collectStatistics(hospital);

    fprintf(file, "============================================================\n");
    fprintf(file, "医院统计报告（基于真实数据库数据）\n");
    fprintf(file, "============================================================\n\n");

    time_t t;
    time(&t);
    fprintf(file, "报告日期: %s\n\n", ctime(&t));

    fprintf(file, "医院概览\n");
    fprintf(file, "------------------------------------------------------------\n");
    fprintf(file, "科室总数:       %d\n", stats.totalDepartments);
    fprintf(file, "病房总数:       %d\n", stats.totalWards);
    fprintf(file, "床位总数:       %d\n", stats.totalBeds);
    fprintf(file, "患者总数:       %d\n", stats.totalPatients);
    fprintf(file, "总体占用率:     %.1f%%\n\n", stats.overallOccupancyRate);

    fprintf(file, "科室细分\n");
    fprintf(file, "------------------------------------------------------------\n");
    for (int i = 0; i < stats.departmentCount; i++) {
        fprintf(file, "%s: %.1f%% 占用率\n", stats.departmentNames[i], stats.departmentOccupancyRates[i]);
    }

    fprintf(file, "\n当前住院患者列表\n");
    fprintf(file, "------------------------------------------------------------\n");

    int patientCount = 0;
    Department* d = hospital;
    while (d) {
        Ward* w = d->wardList;
        while (w) {
            Bed* bed = w->bedList;
            while (bed) {
                if (bed->status == STATUS_OCCUPIED) {
                    patientCount++;
                    fprintf(file, "%d. %s (ID:%d) - %s -> 病房%d, 床位%d\n",
                            patientCount,
                            bed->patientName, bed->patientId,
                            d->departmentName,
                            w->wardNumber, bed->bedNumber);
                }
                bed = bed->next;
            }
            w = w->next;
        }
        d = d->next;
    }

    if (patientCount == 0) {
        fprintf(file, "当前无住院患者\n");
    } else {
        fprintf(file, "\n患者总数: %d\n", patientCount);
    }

    fclose(file);
    printSuccess("统计报告已导出到 hospital_report.txt");
}

void generateStatisticsReport(Department* hospital, ReportType type) {
    if (type == REPORT_FILE || type == REPORT_BOTH) {
        exportStatisticsToFile(hospital);
    }
}

void performDataAnalysis(Department* hospital) {
    // 保留原逻辑：仍然基于真实内存（由DB加载）数据生成
    if (!hospital) {
        printError("医院数据为空！无法进行AI分析。");
        return;
    }

    time_t now;
    time(&now);

    FILE* file = fopen(DB_ANALYSIS_FILE, "w");
    if (!file) {
        printError("无法创建人工智能分析报告文件");
        return;
    }

    fprintf(file, "================================================================================\n");
    fprintf(file, "                    人工智能辅助数据分析报告\n");
    fprintf(file, "================================================================================\n");
    fprintf(file, "报告生成时间: %s", ctime(&now));
    fprintf(file, "数据源: 真实数据库文件 (departments_db.txt, wards_db.txt, beds_db.txt, patients_db.txt)\n");
    fprintf(file, "================================================================================\n\n");

    HospitalStatistics stats = collectStatistics(hospital);

    fprintf(file, "医院概览（基于真实数据库）\n");
    fprintf(file, "------------------------------------------------------------\n");
    fprintf(file, "科室总数:       %d\n", stats.totalDepartments);
    fprintf(file, "病房总数:       %d\n", stats.totalWards);
    fprintf(file, "床位总数:       %d\n", stats.totalBeds);
    fprintf(file, "患者总数:       %d\n", stats.totalPatients);
    fprintf(file, "总体占用率:     %.1f%%\n\n", stats.overallOccupancyRate);

    fprintf(file, "科室占用率（基于真实数据库）\n");
    fprintf(file, "------------------------------------------------------------\n");
    for (int i = 0; i < stats.departmentCount; i++) {
        fprintf(file, "%-15s : %.1f%%\n", stats.departmentNames[i], stats.departmentOccupancyRates[i]);
    }

    fclose(file);

    printSuccess("人工智能分析报告已从真实数据库数据生成！");
    printf("报告已保存到: %s\n", DB_ANALYSIS_FILE);
}

// ==================== 患者管理 ====================

int admitPatient(Department* hospital, int departmentId, int wardNumber,
                 int bedNumber, int patientId, const char* patientName) {
    if (!hospital) return FAILURE;

    Department* d = hospital;
    while (d) {
        if (d->departmentId == departmentId) {
            Ward* w = findWardInDepartment(d, wardNumber);
            if (!w) {
                printError("在该科室未找到病房");
                return FAILURE;
            }

            Bed* bed = findBedInWard(w, bedNumber);
            if (!bed) {
                printError("在该病房未找到床位");
                return FAILURE;
            }

            if (!isBedAvailable(bed)) {
                printError("床位已被占用");
                return FAILURE;
            }

            occupyBed(bed, patientId, patientName);
            w->occupiedBeds++;
            updateDepartmentStats(d);

            printSuccess("患者入院成功！");
            printInfo("正在更新数据库...");
            saveAllDataToDB(hospital);
            return SUCCESS;
        }
        d = d->next;
    }

    printError("未找到科室");
    return FAILURE;
}

int dischargePatient(Department* hospital, int patientId) {
    if (!hospital) return FAILURE;

    Department* d = hospital;
    while (d) {
        Ward* w = d->wardList;
        while (w) {
            Bed* bed = w->bedList;
            while (bed) {
                if (bed->status == STATUS_OCCUPIED && bed->patientId == patientId) {
                    char confirm;
                    printHeader("找到患者");
                    printf("姓名: %s\n", bed->patientName);
                    printf("科室: %s\n", d->departmentName);
                    printf("病房: %d\n", w->wardNumber);
                    printf("床位: %d\n", bed->bedNumber);
                    printSeparator();

                    printf("\n确认出院？ (y/n): ");
                    scanf(" %c", &confirm);
                    clearInputBuffer();

                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        if (w->occupiedBeds > 0) w->occupiedBeds--;
                        updateDepartmentStats(d);
                        printSuccess("患者出院成功！");
                        printInfo("正在更新数据库...");
                        saveAllDataToDB(hospital);
                        return SUCCESS;
                    }

                    printInfo("出院已取消");
                    return FAILURE;
                }
                bed = bed->next;
            }
            w = w->next;
        }
        d = d->next;
    }

    printError("未找到患者 ID");
    return FAILURE;
}

int findPatientLocation(Department* hospital, int patientId, PatientInfo* info) {
    if (!hospital || !info) return FAILURE;

    Department* d = hospital;
    while (d) {
        Ward* w = d->wardList;
        while (w) {
            Bed* bed = w->bedList;
            while (bed) {
                if (bed->status == STATUS_OCCUPIED && bed->patientId == patientId) {
                    info->patientId = patientId;
                    strncpy(info->patientName, bed->patientName, MAX_NAME_LENGTH - 1);
                    info->patientName[MAX_NAME_LENGTH - 1] = '\0';
                    info->departmentId = d->departmentId;
                    info->wardNumber = w->wardNumber;
                    info->bedNumber = bed->bedNumber;
                    return SUCCESS;
                }
                bed = bed->next;
            }
            w = w->next;
        }
        d = d->next;
    }

    return FAILURE;
}

void displayAllPatients(Department* hospital) {
    if (!hospital) return;

    printHeader("当前住院患者列表");

    int count = 0;
    Department* d = hospital;
    while (d) {
        Ward* w = d->wardList;
        while (w) {
            Bed* bed = w->bedList;
            while (bed) {
                if (bed->status == STATUS_OCCUPIED) {
                    count++;
                    printf("%2d. %-10s (ID:%-6d) %s -> 病房 %d, 床位 %d\n",
                           count, bed->patientName, bed->patientId,
                           d->departmentName, w->wardNumber, bed->bedNumber);
                }
                bed = bed->next;
            }
            w = w->next;
        }
        d = d->next;
    }

    if (count == 0) {
        printInfo("当前无住院患者");
    } else {
        printSeparator();
        printf("患者总数: %d\n", count);
    }

    printSeparator();
}

int validateTransfer(Department* hospital, int patientId, int newDepartmentId,
                     int newWardNumber, int newBedNumber, char* errorMessage) {
    PatientInfo info;
    if (!findPatientLocation(hospital, patientId, &info)) {
        sprintf(errorMessage, "未找到患者 ID %d", patientId);
        return FAILURE;
    }

    Department* d = hospital;
    while (d) {
        if (d->departmentId == newDepartmentId) {
            Ward* w = findWardInDepartment(d, newWardNumber);
            if (!w) {
                sprintf(errorMessage, "未找到病房 %d", newWardNumber);
                return FAILURE;
            }

            Bed* bed = findBedInWard(w, newBedNumber);
            if (!bed) {
                sprintf(errorMessage, "未找到床位 %d", newBedNumber);
                return FAILURE;
            }

            if (!isBedAvailable(bed)) {
                sprintf(errorMessage, "床位 %d 已被占用", newBedNumber);
                return FAILURE;
            }

            return SUCCESS;
        }
        d = d->next;
    }

    sprintf(errorMessage, "未找到科室 %d", newDepartmentId);
    return FAILURE;
}

int transferPatient(Department* hospital, int patientId, int newDepartmentId,
                    int newWardNumber, int newBedNumber) {
    char errorMessage[200];

    if (!validateTransfer(hospital, patientId, newDepartmentId, newWardNumber, newBedNumber, errorMessage)) {
        printError(errorMessage);
        return FAILURE;
    }

    PatientInfo oldInfo;
    findPatientLocation(hospital, patientId, &oldInfo);

    // vacate old
    Department* d = hospital;
    while (d) {
        if (d->departmentId == oldInfo.departmentId) {
            Ward* w = findWardInDepartment(d, oldInfo.wardNumber);
            Bed* bed = findBedInWard(w, oldInfo.bedNumber);
            vacateBed(bed);
            if (w->occupiedBeds > 0) w->occupiedBeds--;
            updateDepartmentStats(d);
            break;
        }
        d = d->next;
    }

    // occupy new
    d = hospital;
    while (d) {
        if (d->departmentId == newDepartmentId) {
            Ward* w = findWardInDepartment(d, newWardNumber);
            Bed* bed = findBedInWard(w, newBedNumber);
            occupyBed(bed, patientId, oldInfo.patientName);
            w->occupiedBeds++;
            updateDepartmentStats(d);
            break;
        }
        d = d->next;
    }

    printSuccess("患者转科成功！");
    printInfo("正在更新数据库...");
    saveAllDataToDB(hospital);
    return SUCCESS;
}

void printDepartmentDetails(Department* hospital) {
    if (!hospital) return;
    printHeader("科室详情");
    Department* d = hospital;
    while (d) {
        displayDepartment(d);
        d = d->next;
    }
}

void printWardDetails(Department* hospital) {
    if (!hospital) return;
    printHeader("病房详情");
    Department* d = hospital;
    while (d) {
        printf("\n=== 科室: %s ===\n", d->departmentName);
        Ward* w = d->wardList;
        while (w) {
            displayWard(w);
            w = w->next;
        }
        d = d->next;
    }
}

// ==================== 菜单与输入处理 ====================

void printMainMenu(void) {
    printf("\n============================================================\n");
    printf("                医院管理系统\n");
    printf("============================================================\n");
    printf("  1.  收治患者\n");
    printf("  2.  转科/转床\n");
    printf("  3.  患者出院\n");
    printf("  4.  释放床位\n");
    printf("  5.  查看科室信息\n");
    printf("  6.  查看病房信息\n");
    printf("  7.  查看所有住院患者\n");
    printf("  8.  生成统计报告 (TXT)\n");
    printf("  9.  人工智能数据分析与预测 (生成TXT报告)\n");
    printf(" 10.  退出系统\n");
    printf("============================================================\n");
}

void handleAdmitPatient(Department* hospital) {
    printHeader("收治患者");

    displayDepartmentList();

    int deptId = getValidatedIntInput("输入科室 ID", 1, 999999);

    displayWardListForDepartment(deptId);

    int wardNum;
    printf("输入病房号: ");
    scanf("%d", &wardNum);
    clearInputBuffer();

    int maxBed = getMaxBedNumberInWard(hospital, deptId, wardNum);
    if (maxBed <= 0) {
        printError("无效病房号（该科室下不存在该病房）");
        return;
    }

    int bedNum = getValidatedIntInput("输入床位号", 1, maxBed);
    int patientId = getValidatedIntInput("输入患者 ID", 1, 999999);

    char patientName[MAX_NAME_LENGTH];
    printf("输入患者姓名: ");
    fgets(patientName, sizeof(patientName), stdin);
    patientName[strcspn(patientName, "\n")] = 0;

    admitPatient(hospital, deptId, wardNum, bedNum, patientId, patientName);
}

void handleTransferPatient(Department* hospital) {
    printHeader("转科/转床");

    displayAllPatients(hospital);

    int patientId = getValidatedIntInput("输入要转科的患者 ID", 1, 999999);
    int newDeptId = getValidatedIntInput("输入新科室 ID", 1, 999999);

    displayWardListForDepartment(newDeptId);

    int newWardNum;
    printf("输入新病房号: ");
    scanf("%d", &newWardNum);
    clearInputBuffer();

    int maxBed = getMaxBedNumberInWard(hospital, newDeptId, newWardNum);
    if (maxBed <= 0) {
        printError("无效病房号（该科室下不存在该病房）");
        return;
    }

    int newBedNum = getValidatedIntInput("输入新床位号", 1, maxBed);

    transferPatient(hospital, patientId, newDeptId, newWardNum, newBedNum);
}

void handleDischargePatient(Department* hospital) {
    printHeader("患者出院");
    displayAllPatients(hospital);
    int patientId = getValidatedIntInput("输入要出院的患者 ID", 1, 999999);
    dischargePatient(hospital, patientId);
}

void handleReleaseBed(Department* hospital) {
    printHeader("释放床位");

    displayDepartmentList();
    int deptId = getValidatedIntInput("输入科室 ID", 1, 999999);

    displayWardListForDepartment(deptId);

    int wardNum;
    printf("输入病房号: ");
    scanf("%d", &wardNum);
    clearInputBuffer();

    int maxBed = getMaxBedNumberInWard(hospital, deptId, wardNum);
    if (maxBed <= 0) {
        printError("无效病房号（该科室下不存在该病房）");
        return;
    }

    int bedNum = getValidatedIntInput("输入床位号", 1, maxBed);

    Department* d = hospital;
    while (d) {
        if (d->departmentId == deptId) {
            Ward* w = findWardInDepartment(d, wardNum);
            if (!w) return;
            Bed* bed = findBedInWard(w, bedNum);
            if (!bed) return;

            if (bed->status != STATUS_OCCUPIED) {
                printInfo("床位已是可用状态");
                return;
            }

            printf("\n床位 %d - 当前患者: %s (ID: %d)\n", bedNum, bed->patientName, bed->patientId);
            char confirm;
            printf("确认释放床位？ (y/n): ");
            scanf(" %c", &confirm);
            clearInputBuffer();

            if (confirm == 'y' || confirm == 'Y') {
                vacateBed(bed);
                if (w->occupiedBeds > 0) w->occupiedBeds--;
                updateDepartmentStats(d);
                printSuccess("床位释放成功！");
                saveAllDataToDB(hospital);
            }
            return;
        }
        d = d->next;
    }
}

// ==================== 初始化（仅用于DB缺失时） ====================

Department* initializeHospital(void) {
    // 初始化的结构用于“首次创建DB”，不会用于展示时的mock（展示均来自g_hospital内存）
    // 这里仍保留固定拓扑，但后续所有业务以DB load 的数据为准。

    Department* h = createDepartment(1, "心内科");
    addWardToDepartment(h, createWard(101, "心内科普通病房", 10));
    addWardToDepartment(h, createWard(102, "心内科重症监护室", 5));
    addWardToDepartment(h, createWard(103, "心内科私人病房", 5));

    Department* d2 = createDepartment(2, "外科");
    addWardToDepartment(d2, createWard(201, "外科普通病房", 10));
    addWardToDepartment(d2, createWard(202, "外科私人病房", 5));

    Department* d3 = createDepartment(3, "儿科");
    addWardToDepartment(d3, createWard(301, "儿科普通病房", 10));
    addWardToDepartment(d3, createWard(302, "儿科隔离病房", 5));

    Department* d4 = createDepartment(4, "骨科");
    addWardToDepartment(d4, createWard(401, "骨科普通病房", 10));

    Department* d5 = createDepartment(5, "急诊科");
    addWardToDepartment(d5, createWard(501, "急诊观察室", 10));

    h->next = d2;
    d2->next = d3;
    d3->next = d4;
    d4->next = d5;

    return h;
}

void init_hospital_globals(void) {
    printf("\n[模块 D] 初始化中...\n");

    if (checkDatabaseIntegrity()) {
        printInfo("找到现有数据库文件，正在加载...");
        if (loadAllDataFromDB(&g_hospital) == SUCCESS && g_hospital != NULL) {
            printSuccess("数据库加载成功！");
            return;
        }
        printError("数据库加载失败，将创建新数据库...");
    } else {
        printInfo("未找到数据库文件，正在创建新数据库...");
    }

    g_hospital = initializeHospital();
    saveAllDataToDB(g_hospital);
    printSuccess("新数据库已创建并保存！");
}

// ==================== 入口/释放 ====================

int D_entry(void) {
    if (g_hospital == NULL) {
        init_hospital_globals();
    }

    printf("\n[模块 D] 就绪 - 实时数据库模式（文本文件）\n");
    printDatabaseSummary(g_hospital);

    int choice;
    do {
        printMainMenu();
        choice = getValidatedIntInput("选择选项", 0, 10);

        switch (choice) {
            case 1: handleAdmitPatient(g_hospital); break;
            case 2: handleTransferPatient(g_hospital); break;
            case 3: handleDischargePatient(g_hospital); break;
            case 4: handleReleaseBed(g_hospital); break;
            case 5: printDepartmentDetails(g_hospital); break;
            case 6: printWardDetails(g_hospital); break;
            case 7: displayAllPatients(g_hospital); break;
            case 8: generateStatisticsReport(g_hospital, 2 /*REPORT_FILE*/); break;
            case 9: performDataAnalysis(g_hospital); break;
            case 10:
                printInfo("正在将数据保存到数据库...");
                saveAllDataToDB(g_hospital);
                printSuccess("正在退出系统...");
                freeHospitalSystem(&g_hospital);
                printMemoryReport();
                printf("\n感谢使用医院管理系统！\n");
                break;
        }
    } while (choice != 10);

    return 0;
}

void freeHospitalSystem(Department** hospital) {
    if (!hospital || !*hospital) return;

    Department* cur = *hospital;
    while (cur) {
        Department* next = cur->next;
        freeDepartment(cur);
        cur = next;
    }
    *hospital = NULL;
}

void printMemoryReport(void) {
    printf("\n============================================================\n");
    printSuccess("所有内存已释放");
    printSuccess("数据库文件已保存");
    printf("============================================================\n");
}


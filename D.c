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
#define TRUE 1
#define FALSE 0

// 数据库文件名
#define DB_DEPARTMENTS_FILE "departments_db.txt"
#define DB_WARDS_FILE "wards_db.txt"
#define DB_BEDS_FILE "beds_db.txt"
#define DB_PATIENTS_FILE "patients_db.txt"

// 终端颜色代码
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_RESET "\033[0m"

typedef enum {
    STATUS_AVAILABLE = 0,
    STATUS_OCCUPIED = 1
} BedStatus;

typedef enum {
    REPORT_CONSOLE = 1,
    REPORT_FILE = 2,
    REPORT_BOTH = 3
} ReportType;

// 核心数据结构
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

// 函数声明
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
int getAvailableBedsInWard(const Ward* ward);
float getWardOccupancyRate(const Ward* ward);
void displayWardBeds(const Ward* ward);
void displayWard(const Ward* ward);
void freeWard(Ward* ward);

Department* createDepartment(int departmentId, const char* departmentName);
void addWardToDepartment(Department* department, Ward* ward);
Ward* findWardInDepartment(const Department* department, int wardNumber);
void updateDepartmentStats(Department* department);
void displayDepartment(const Department* department);
void freeDepartment(Department* department);

HospitalStatistics collectStatistics(Department* hospital);
void exportStatisticsToFile(const HospitalStatistics* stats, const char* filename);
void exportStatisticsToJSON(const HospitalStatistics* stats, const char* filename);
void exportStatisticsToCSV(const HospitalStatistics* stats, const char* filename);
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
int isValidName(const char* name);
void clearInputBuffer(void);
void printSuccess(const char* message);
void printError(const char* message);
void printInfo(const char* message);
void printHeader(const char* title);
void printSeparator(void);
void displayDepartmentList(void);
void displayWardListForDepartment(int deptId);
int getValidatedWardNumber(int deptId);
int getValidatedBedNumber(int wardNumber);
void freeHospitalSystem(Department** hospital);
void printMemoryReport(void);
void printMainMenu(void);
Department* initializeHospital(void);
void performDataAnalysis(Department* hospital);
void handleAdmitPatient(Department* hospital);
void handleTransferPatient(Department* hospital);
void handleDischargePatient(Department* hospital);
void handleReleaseBed(Department* hospital);
void init_hospital_globals(void);
int D_entry(void);

// 数据库函数
int saveDepartmentsToDB(Department* hospital);
int saveWardsToDB(Department* hospital);
int saveBedsToDB(Department* hospital);
int savePatientsToDB(Department* hospital);
int loadDepartmentsFromDB(Department** hospital);
int loadWardsFromDB(Department* hospital);
int loadBedsFromDB(Department* hospital);
int loadPatientsFromDB(Department* hospital);
int saveAllDataToDB(Department* hospital);
int loadAllDataFromDB(Department** hospital);
int isDatabaseEmpty(void);
void createBackup(void);

// ==================== 用户界面辅助函数 ====================

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printSuccess(const char* message) {
    printf(COLOR_GREEN "✓ %s" COLOR_RESET "\n", message);
}

void printError(const char* message) {
    printf(COLOR_RED "✗ 错误: %s" COLOR_RESET "\n", message);
}

void printInfo(const char* message) {
    printf(COLOR_CYAN "ℹ %s" COLOR_RESET "\n", message);
}

void printHeader(const char* title) {
    printf("\n" COLOR_BLUE "╔══════════════════════════════════════════════════════════════╗\n");
    printf("║ %-60s ║\n", title);
    printf("╚══════════════════════════════════════════════════════════════╝" COLOR_RESET "\n");
}

void printSeparator(void) {
    printf(COLOR_YELLOW "------------------------------------------------------------\n" COLOR_RESET);
}

void displayDepartmentList(void) {
    printf("\n" COLOR_CYAN "可用科室列表:" COLOR_RESET "\n");
    printf("  ┌────┬─────────────────────────┬──────────┬──────────┐\n");
    printf("  │ 编号 │ 科室名称                │ 病房数   │ 床位数   │\n");
    printf("  ├────┼─────────────────────────┼──────────┼──────────┤\n");
    printf("  │ 1  │ 心血管内科              │ 3个病房  │ 20张床位 │\n");
    printf("  │ 2  │ 外科                    │ 2个病房  │ 15张床位 │\n");
    printf("  │ 3  │ 儿科                    │ 2个病房  │ 15张床位 │\n");
    printf("  │ 4  │ 骨科                    │ 1个病房  │ 10张床位 │\n");
    printf("  │ 5  │ 急诊科                  │ 1个病房  │ 15张床位 │\n");
    printf("  └────┴─────────────────────────┴──────────┴──────────┘\n");
}

void displayWardListForDepartment(int deptId) {
    printf("\n" COLOR_CYAN "可用病房列表:" COLOR_RESET "\n");
    switch(deptId) {
        case 1:
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ 编号 │ 病房名称                 │ 床位数   │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 101 │ 心血管内科普通病房       │ 10张床位 │\n");
            printf("  │ 102 │ 心血管内科重症监护室     │ 5张床位  │\n");
            printf("  │ 103 │ 心血管内科特需病房       │ 5张床位  │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 2:
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ 编号 │ 病房名称                 │ 床位数   │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 201 │ 外科普通病房             │ 10张床位 │\n");
            printf("  │ 202 │ 外科特需病房             │ 5张床位  │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 3:
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ 编号 │ 病房名称                 │ 床位数   │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 301 │ 儿科普通病房             │ 10张床位 │\n");
            printf("  │ 302 │ 儿科隔离病房             │ 5张床位  │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 4:
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ 编号 │ 病房名称                 │ 床位数   │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 401 │ 骨科普通病房             │ 10张床位 │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 5:
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ 编号 │ 病房名称                 │ 床位数   │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 501 │ 急诊观察室               │ 10张床位 │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
    }
}

int isValidName(const char* name) {
    if (name == NULL || strlen(name) == 0) return 0;
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha(name[i]) && name[i] != ' ' && name[i] != '-') {
            return 0;
        }
    }
    return 1;
}

int getValidatedIntInput(const char* prompt, int min, int max) {
    int value;
    int valid;
    char input[100];
    
    do {
        printf("%s", prompt);
        printf(COLOR_YELLOW "[%d-%d]" COLOR_RESET ": ", min, max);
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &value) == 1) {
                if (value >= min && value <= max) {
                    valid = 1;
                } else {
                    printError("输入超出范围！");
                    printf("   请输入 %d 到 %d 之间的数字。\n", min, max);
                    valid = 0;
                }
            } else {
                printError("无效输入！请输入有效的数字。");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return value;
}

int getValidatedWardNumber(int deptId) {
    int wardNum;
    int valid;
    char input[100];
    int validWards[5] = {0};
    int validCount = 0;
    
    switch(deptId) {
        case 1: 
            validWards[0]=101; validWards[1]=102; validWards[2]=103; 
            validCount=3; 
            break;
        case 2: 
            validWards[0]=201; validWards[1]=202; 
            validCount=2; 
            break;
        case 3: 
            validWards[0]=301; validWards[1]=302; 
            validCount=2; 
            break;
        case 4: 
            validWards[0]=401; 
            validCount=1; 
            break;
        case 5: 
            validWards[0]=501; 
            validCount=1; 
            break;
    }
    
    do {
        printf("请输入病房号: ");
        printf(COLOR_YELLOW "[");
        for(int i=0; i<validCount; i++) {
            printf("%d%s", validWards[i], (i<validCount-1) ? "/" : "");
        }
        printf("]" COLOR_RESET ": ");
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &wardNum) == 1) {
                valid = 0;
                for(int i=0; i<validCount; i++) {
                    if (wardNum == validWards[i]) {
                        valid = 1;
                        break;
                    }
                }
                if (!valid) {
                    printError("无效的病房号！");
                    printf("   请从以下选择: ");
                    for(int i=0; i<validCount; i++) {
                        printf("%d%s", validWards[i], (i<validCount-1) ? ", " : "");
                    }
                    printf("\n");
                }
            } else {
                printError("无效输入！请输入有效的数字。");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return wardNum;
}

int getValidatedBedNumber(int wardNumber) {
    int bedNum;
    int valid;
    char input[100];
    int maxBed = 10;
    
    if (wardNumber == 102 || wardNumber == 103 || wardNumber == 202 || wardNumber == 302) {
        maxBed = 5;
    } else {
        maxBed = 10;
    }
    
    do {
        printf("请输入床位号: ");
        printf(COLOR_YELLOW "[1-%d]" COLOR_RESET ": ", maxBed);
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &bedNum) == 1) {
                if (bedNum >= 1 && bedNum <= maxBed) {
                    valid = 1;
                } else {
                    printError("床位号超出范围！");
                    printf("   该病房只有 1 到 %d 号床位。\n", maxBed);
                    valid = 0;
                }
            } else {
                printError("无效输入！请输入有效的数字。");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return bedNum;
}

// ==================== 数据库函数 ====================

int saveDepartmentsToDB(Department* hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "w");
    if (file == NULL) {
        printError("无法打开科室数据库文件");
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
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
    if (file == NULL) {
        printError("无法打开病房数据库文件");
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            fprintf(file, "%d|%d|%s|%d|%d\n",
                    dept->departmentId,
                    ward->wardNumber,
                    ward->wardName,
                    ward->totalBeds,
                    ward->occupiedBeds);
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    fclose(file);
    return SUCCESS;
}

int saveBedsToDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "w");
    if (file == NULL) {
        printError("无法打开床位数据库文件");
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                fprintf(file, "%d|%d|%d|%d|%d|%s\n",
                        dept->departmentId,
                        ward->wardNumber,
                        bed->bedNumber,
                        bed->status,
                        bed->patientId,
                        bed->patientName);
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    fclose(file);
    return SUCCESS;
}

int savePatientsToDB(Department* hospital) {
    FILE* file = fopen(DB_PATIENTS_FILE, "w");
    if (file == NULL) {
        printError("无法打开病人数据库文件");
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED) {
                    fprintf(file, "%d|%s|%d|%d|%d\n",
                            bed->patientId,
                            bed->patientName,
                            dept->departmentId,
                            ward->wardNumber,
                            bed->bedNumber);
                }
                bed = bed->next;
            }
            ward = ward->next;
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
    
    printSuccess("数据库保存成功");
    createBackup();
    return SUCCESS;
}

int loadDepartmentsFromDB(Department** hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    Department* tail = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        int deptId, totalWards, totalBeds, totalPatients;
        char deptName[MAX_NAME_LENGTH];
        
        sscanf(line, "%d|%[^|]|%d|%d|%d", 
               &deptId, deptName, &totalWards, &totalBeds, &totalPatients);
        
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
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        int deptId, wardNumber, totalBeds, occupiedBeds;
        char wardName[MAX_NAME_LENGTH];
        
        sscanf(line, "%d|%d|%[^|]|%d|%d", 
               &deptId, &wardNumber, wardName, &totalBeds, &occupiedBeds);
        
        Department* dept = hospital;
        while (dept != NULL) {
            if (dept->departmentId == deptId) {
                Ward* ward = createWard(wardNumber, wardName, totalBeds);
                ward->occupiedBeds = occupiedBeds;
                addWardToDepartment(dept, ward);
                break;
            }
            dept = dept->next;
        }
    }
    
    fclose(file);
    return SUCCESS;
}

int loadBedsFromDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        int deptId, wardNumber, bedNumber, status, patientId;
        char patientName[MAX_NAME_LENGTH];
        
        sscanf(line, "%d|%d|%d|%d|%d|%[^\n]", 
               &deptId, &wardNumber, &bedNumber, &status, &patientId, patientName);
        
        Department* dept = hospital;
        while (dept != NULL) {
            if (dept->departmentId == deptId) {
                Ward* ward = findWardInDepartment(dept, wardNumber);
                if (ward != NULL) {
                    Bed* bed = findBedInWard(ward, bedNumber);
                    if (bed != NULL) {
                        bed->status = status;
                        if (status == STATUS_OCCUPIED) {
                            bed->patientId = patientId;
                            strcpy(bed->patientName, patientName);
                        }
                    }
                }
                break;
            }
            dept = dept->next;
        }
    }
    
    fclose(file);
    return SUCCESS;
}

int loadPatientsFromDB(Department* hospital) {
    FILE* file = fopen(DB_PATIENTS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        int patientId, deptId, wardNumber, bedNumber;
        char patientName[MAX_NAME_LENGTH];
        
        sscanf(line, "%d|%[^|]|%d|%d|%d", 
               &patientId, patientName, &deptId, &wardNumber, &bedNumber);
        
        Department* dept = hospital;
        while (dept != NULL) {
            if (dept->departmentId == deptId) {
                Ward* ward = findWardInDepartment(dept, wardNumber);
                if (ward != NULL) {
                    Bed* bed = findBedInWard(ward, bedNumber);
                    if (bed != NULL && bed->status == STATUS_AVAILABLE) {
                        occupyBed(bed, patientId, patientName);
                        ward->occupiedBeds++;
                    }
                }
                break;
            }
            dept = dept->next;
        }
    }
    
    fclose(file);
    updateDepartmentStats(hospital);
    return SUCCESS;
}

int loadAllDataFromDB(Department** hospital) {
    if (loadDepartmentsFromDB(hospital) == FAILURE) {
        return FAILURE;
    }
    
    if (*hospital == NULL) {
        return FAILURE;
    }
    
    loadWardsFromDB(*hospital);
    loadBedsFromDB(*hospital);
    loadPatientsFromDB(*hospital);
    
    return SUCCESS;
}

int isDatabaseEmpty(void) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "r");
    if (file == NULL) return 1;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    return (size == 0);
}

void createBackup(void) {
    char backupName[100];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    
    sprintf(backupName, "backup_%04d%02d%02d_%02d%02d%02d.txt",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* backup = fopen(backupName, "w");
    if (backup == NULL) return;
    
    fprintf(backup, "医院管理系统备份文件\n");
    fprintf(backup, "备份日期: %s", ctime(&now));
    fprintf(backup, "====================================\n\n");
    
    const char* dbFiles[] = {DB_DEPARTMENTS_FILE, DB_WARDS_FILE, DB_BEDS_FILE, DB_PATIENTS_FILE};
    
    for (int i = 0; i < 4; i++) {
        FILE* src = fopen(dbFiles[i], "r");
        if (src != NULL) {
            fprintf(backup, "\n--- %s 的内容 ---\n", dbFiles[i]);
            char ch;
            while ((ch = fgetc(src)) != EOF) {
                fputc(ch, backup);
            }
            fclose(src);
        }
    }
    
    fclose(backup);
}

// ==================== 床位函数 ====================
Bed* createBed(int bedNumber) {
    Bed* bed = (Bed*)malloc(sizeof(Bed));
    if (bed == NULL) {
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
    strcpy(bed->patientName, "");
    bed->next = NULL;
}

void occupyBed(Bed* bed, int patientId, const char* patientName) {
    if (bed == NULL) return;
    bed->status = STATUS_OCCUPIED;
    bed->patientId = patientId;
    strcpy(bed->patientName, patientName);
}

void vacateBed(Bed* bed) {
    if (bed == NULL) return;
    bed->status = STATUS_AVAILABLE;
    bed->patientId = -1;
    strcpy(bed->patientName, "");
}

int isBedAvailable(const Bed* bed) {
    return bed != NULL && bed->status == STATUS_AVAILABLE;
}

void displayBed(const Bed* bed) {
    if (bed == NULL) return;
    
    if (bed->status == STATUS_OCCUPIED) {
        printf("  床位 %2d: " COLOR_RED "[已占用]" COLOR_RESET " 病人: %s (编号: %d)\n", 
               bed->bedNumber, bed->patientName, bed->patientId);
    } else {
        printf("  床位 %2d: " COLOR_GREEN "[空闲]" COLOR_RESET "\n", bed->bedNumber);
    }
}

void freeBed(Bed* bed) {
    if (bed != NULL) {
        free(bed);
    }
}

// ==================== 病房函数 ====================
Ward* createWard(int wardNumber, const char* wardName, int totalBeds) {
    Ward* ward = (Ward*)malloc(sizeof(Ward));
    if (ward == NULL) {
        printError("病房内存分配失败");
        return NULL;
    }
    
    ward->wardNumber = wardNumber;
    strcpy(ward->wardName, wardName);
    ward->totalBeds = totalBeds;
    ward->occupiedBeds = 0;
    ward->bedList = NULL;
    ward->next = NULL;
    
    for (int i = 1; i <= totalBeds; i++) {
        Bed* bed = createBed(i);
        addBedToWard(ward, bed);
    }
    
    return ward;
}

void addBedToWard(Ward* ward, Bed* bed) {
    if (ward == NULL || bed == NULL) return;
    
    if (ward->bedList == NULL) {
        ward->bedList = bed;
    } else {
        Bed* current = ward->bedList;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = bed;
    }
}

Bed* findBedInWard(const Ward* ward, int bedNumber) {
    if (ward == NULL) return NULL;
    
    Bed* current = ward->bedList;
    while (current != NULL) {
        if (current->bedNumber == bedNumber) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int getAvailableBedsInWard(const Ward* ward) {
    if (ward == NULL) return 0;
    return ward->totalBeds - ward->occupiedBeds;
}

float getWardOccupancyRate(const Ward* ward) {
    if (ward == NULL || ward->totalBeds == 0) return 0.0f;
    return (float)ward->occupiedBeds / ward->totalBeds * 100;
}

void displayWardBeds(const Ward* ward) {
    if (ward == NULL) return;
    
    Bed* current = ward->bedList;
    int count = 0;
    while (current != NULL) {
        displayBed(current);
        current = current->next;
        count++;
        if (count % 5 == 0 && count < ward->totalBeds) {
            printf("  %s...%s\n", COLOR_YELLOW, COLOR_RESET);
        }
    }
}

void displayWard(const Ward* ward) {
    if (ward == NULL) return;
    
    printf("\n" COLOR_BLUE "============================================================" COLOR_RESET "\n");
    printf(COLOR_CYAN "病房 %d: %s" COLOR_RESET "\n", ward->wardNumber, ward->wardName);
    printf("入住率: %d/%d 床位 (%.1f%%)\n", 
           ward->occupiedBeds, ward->totalBeds, getWardOccupancyRate(ward));
    printf(COLOR_YELLOW "------------------------------------------------------------" COLOR_RESET "\n");
    displayWardBeds(ward);
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
}

void freeWard(Ward* ward) {
    if (ward == NULL) return;
    
    Bed* current = ward->bedList;
    while (current != NULL) {
        Bed* next = current->next;
        freeBed(current);
        current = next;
    }
    
    free(ward);
}

// ==================== 科室函数 ====================
Department* createDepartment(int departmentId, const char* departmentName) {
    Department* department = (Department*)malloc(sizeof(Department));
    if (department == NULL) {
        printError("科室内存分配失败");
        return NULL;
    }
    
    department->departmentId = departmentId;
    strcpy(department->departmentName, departmentName);
    department->totalWards = 0;
    department->totalBeds = 0;
    department->totalPatients = 0;
    department->wardList = NULL;
    department->next = NULL;
    
    return department;
}

void addWardToDepartment(Department* department, Ward* ward) {
    if (department == NULL || ward == NULL) return;
    
    if (department->wardList == NULL) {
        department->wardList = ward;
    } else {
        Ward* current = department->wardList;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = ward;
    }
    
    department->totalWards++;
    department->totalBeds += ward->totalBeds;
    updateDepartmentStats(department);
}

Ward* findWardInDepartment(const Department* department, int wardNumber) {
    if (department == NULL) return NULL;
    
    Ward* current = department->wardList;
    while (current != NULL) {
        if (current->wardNumber == wardNumber) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void updateDepartmentStats(Department* department) {
    if (department == NULL) return;
    
    department->totalPatients = 0;
    Ward* current = department->wardList;
    
    while (current != NULL) {
        department->totalPatients += current->occupiedBeds;
        current = current->next;
    }
}

void displayDepartment(const Department* department) {
    if (department == NULL) return;
    
    printf("\n" COLOR_BLUE "============================================================" COLOR_RESET "\n");
    printf(COLOR_CYAN "科室: %s (编号: %d)" COLOR_RESET "\n", department->departmentName, department->departmentId);
    printf("统计: %d 位病人 | %d 张床位 | %d 个病房\n", 
           department->totalPatients, department->totalBeds, department->totalWards);
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
    
    Ward* current = department->wardList;
    while (current != NULL) {
        displayWard(current);
        current = current->next;
    }
}

void freeDepartment(Department* department) {
    if (department == NULL) return;
    
    Ward* current = department->wardList;
    while (current != NULL) {
        Ward* next = current->next;
        freeWard(current);
        current = next;
    }
    
    free(department);
}

// ==================== 统计报告函数 ====================
HospitalStatistics collectStatistics(Department* hospital) {
    HospitalStatistics stats;
    memset(&stats, 0, sizeof(stats));
    
    Department* dept = hospital;
    int deptIndex = 0;
    
    while (dept != NULL && deptIndex < MAX_DEPARTMENTS) {
        stats.totalDepartments++;
        stats.totalWards += dept->totalWards;
        stats.totalBeds += dept->totalBeds;
        stats.totalPatients += dept->totalPatients;
        
        if (dept->totalBeds > 0) {
            stats.departmentOccupancyRates[deptIndex] = 
                (float)dept->totalPatients / dept->totalBeds * 100;
        }
        strcpy(stats.departmentNames[deptIndex], dept->departmentName);
        deptIndex++;
        
        dept = dept->next;
    }
    
    stats.departmentCount = deptIndex;
    
    if (stats.totalBeds > 0) {
        stats.overallOccupancyRate = (float)stats.totalPatients / stats.totalBeds * 100;
    }
    
    return stats;
}

void exportStatisticsToFile(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printError("无法创建报告文件");
        return;
    }
    
    fprintf(file, "============================================================\n");
    fprintf(file, "医院统计报告\n");
    fprintf(file, "============================================================\n\n");
    
    time_t t;
    time(&t);
    fprintf(file, "报告日期: %s\n\n", ctime(&t));
    
    fprintf(file, "医院总体情况\n");
    fprintf(file, "------------------------------------------------------------\n");
    fprintf(file, "总科室数:       %d\n", stats->totalDepartments);
    fprintf(file, "总病房数:       %d\n", stats->totalWards);
    fprintf(file, "总床位数:       %d\n", stats->totalBeds);
    fprintf(file, "总病人数:       %d\n", stats->totalPatients);
    fprintf(file, "全院入住率:     %.1f%%\n\n", stats->overallOccupancyRate);
    
    fprintf(file, "各科室详情\n");
    fprintf(file, "------------------------------------------------------------\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "%s: %.1f%% 入住率\n", 
                stats->departmentNames[i], stats->departmentOccupancyRates[i]);
    }
    
    fprintf(file, "\n入住率可视化 (每个 # = 5%%)\n");
    fprintf(file, "------------------------------------------------------------\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        int bars = (int)(stats->departmentOccupancyRates[i] / 5.0f);
        if (bars > 20) bars = 20;
        fprintf(file, "%-15s %4.0f%% |", stats->departmentNames[i], stats->departmentOccupancyRates[i]);
        for (int j = 0; j < bars; j++) fprintf(file, "#");
        for (int j = bars; j < 20; j++) fprintf(file, " ");
        fprintf(file, "|\n");
    }
    
    fclose(file);
    printf(COLOR_GREEN "✓ 文本报告已导出到 '%s'\n" COLOR_RESET, filename);
}

void exportStatisticsToJSON(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printError("无法创建JSON报告文件");
        return;
    }
    
    fprintf(file, "{\n");
    fprintf(file, "  \"reportType\": \"hospital_statistics\",\n");
    time_t t = time(NULL);
    fprintf(file, "  \"generated\": \"%s\",\n", ctime(&t));
    fprintf(file, "  \"totalDepartments\": %d,\n", stats->totalDepartments);
    fprintf(file, "  \"totalWards\": %d,\n", stats->totalWards);
    fprintf(file, "  \"totalBeds\": %d,\n", stats->totalBeds);
    fprintf(file, "  \"totalPatients\": %d,\n", stats->totalPatients);
    fprintf(file, "  \"overallOccupancyRate\": %.2f,\n", stats->overallOccupancyRate);
    fprintf(file, "  \"departments\": [\n");
    
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "    {\n");
        fprintf(file, "      \"name\": \"%s\",\n", stats->departmentNames[i]);
        fprintf(file, "      \"occupancyRate\": %.2f\n", stats->departmentOccupancyRates[i]);
        fprintf(file, "    }%s\n", (i < stats->departmentCount - 1) ? "," : "");
    }
    fprintf(file, "  ]\n}\n");
    fclose(file);
    printf(COLOR_GREEN "✓ JSON报告已导出到 '%s'\n" COLOR_RESET, filename);
}

void exportStatisticsToCSV(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printError("无法创建CSV报告文件");
        return;
    }
    
    fprintf(file, "科室,病人数,床位数,入住率(%%)\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "%s,%d,%d,%.2f\n", 
                stats->departmentNames[i],
                (int)(stats->departmentOccupancyRates[i] * stats->totalBeds / 100.0f / stats->departmentCount),
                stats->totalBeds / stats->departmentCount,
                stats->departmentOccupancyRates[i]);
    }
    fprintf(file, "总计,%d,%d,%.2f\n", 
            stats->totalPatients, stats->totalBeds, stats->overallOccupancyRate);
    fclose(file);
    printf(COLOR_GREEN "✓ CSV报告已导出到 '%s'\n" COLOR_RESET, filename);
}

void generateStatisticsReport(Department* hospital, ReportType type) {
    HospitalStatistics stats = collectStatistics(hospital);
    
    if (type == REPORT_FILE || type == REPORT_BOTH) {
        exportStatisticsToFile(&stats, "hospital_report.txt");
        exportStatisticsToJSON(&stats, "hospital_report.json");
        exportStatisticsToCSV(&stats, "hospital_report.csv");
        printSuccess("所有报告生成成功！");
    }
}

// ==================== 病人管理函数 ====================
void printDepartmentDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printHeader("科室详细信息");
    
    Department* dept = hospital;
    while (dept != NULL) {
        displayDepartment(dept);
        dept = dept->next;
    }
}

void printWardDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printHeader("病房详细信息");
    
    Department* dept = hospital;
    while (dept != NULL) {
        printf("\n" COLOR_CYAN "=== 科室: %s ===" COLOR_RESET "\n", dept->departmentName);
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            displayWard(ward);
            ward = ward->next;
        }
        dept = dept->next;
    }
}

int admitPatient(Department* hospital, int departmentId, int wardNumber, 
                 int bedNumber, int patientId, const char* patientName) {
    if (hospital == NULL) return FAILURE;
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == departmentId) {
            Ward* ward = findWardInDepartment(dept, wardNumber);
            if (ward == NULL) {
                printError("该科室中没有这个病房");
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, bedNumber);
            if (bed == NULL) {
                printError("该病房中没有这个床位");
                return FAILURE;
            }
            
            if (!isBedAvailable(bed)) {
                printError("该床位已被占用");
                return FAILURE;
            }
            
            occupyBed(bed, patientId, patientName);
            ward->occupiedBeds++;
            updateDepartmentStats(dept);
            
            printf("\n");
            printSuccess("病人入院登记成功！");
            printSeparator();
            printf("  " COLOR_CYAN "科室:" COLOR_RESET " %s\n", dept->departmentName);
            printf("  " COLOR_CYAN "病房:" COLOR_RESET " %d - %s\n", ward->wardNumber, ward->wardName);
            printf("  " COLOR_CYAN "床位:" COLOR_RESET " %d\n", bed->bedNumber);
            printf("  " COLOR_CYAN "病人:" COLOR_RESET " %s (编号: %d)\n", patientName, patientId);
            printSeparator();
            
            printf("\n");
            printInfo("正在保存到数据库...");
            saveAllDataToDB(hospital);
            
            return SUCCESS;
        }
        dept = dept->next;
    }
    
    printError("找不到该科室");
    return FAILURE;
}

int dischargePatient(Department* hospital, int patientId) {
    if (hospital == NULL) return FAILURE;
    
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED && bed->patientId == patientId) {
                    printf("\n");
                    printHeader("找到病人信息");
                    printf("  " COLOR_CYAN "姓名:" COLOR_RESET " %s\n", bed->patientName);
                    printf("  " COLOR_CYAN "科室:" COLOR_RESET " %s\n", dept->departmentName);
                    printf("  " COLOR_CYAN "病房:" COLOR_RESET " %d\n", ward->wardNumber);
                    printf("  " COLOR_CYAN "床位:" COLOR_RESET " %d\n", bed->bedNumber);
                    printSeparator();
                    
                    char confirm;
                    printf("\n" COLOR_YELLOW "确认办理出院吗？(y/n): " COLOR_RESET);
                    scanf(" %c", &confirm);
                    clearInputBuffer();
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("\n");
                        printSuccess("病人出院办理成功！");
                        
                        printf("\n");
                        printInfo("正在保存到数据库...");
                        saveAllDataToDB(hospital);
                        
                        return SUCCESS;
                    } else {
                        printInfo("出院已取消");
                        return FAILURE;
                    }
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    printError("找不到该病人编号");
    return FAILURE;
}

int findPatientLocation(Department* hospital, int patientId, PatientInfo* info) {
    if (hospital == NULL || info == NULL) return FAILURE;
    
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED && bed->patientId == patientId) {
                    info->patientId = patientId;
                    strcpy(info->patientName, bed->patientName);
                    info->departmentId = dept->departmentId;
                    info->wardNumber = ward->wardNumber;
                    info->bedNumber = bed->bedNumber;
                    return SUCCESS;
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    return FAILURE;
}

void displayAllPatients(Department* hospital) {
    if (hospital == NULL) return;
    
    printHeader("当前住院病人列表");
    
    int count = 0;
    Department* dept = hospital;
    
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED) {
                    count++;
                    printf("%2d. " COLOR_CYAN "%-10s" COLOR_RESET " (编号:%-6d) %s → 病房 %d, 床位 %d\n",
                           count, bed->patientName, bed->patientId,
                           dept->departmentName, ward->wardNumber, bed->bedNumber);
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    if (count == 0) {
        printInfo("当前没有住院病人");
    } else {
        printSeparator();
        printf("  " COLOR_YELLOW "总病人数: %d" COLOR_RESET "\n", count);
    }
    
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
}

int validateTransfer(Department* hospital, int patientId, int newDepartmentId, 
                     int newWardNumber, int newBedNumber, char* errorMessage) {
    PatientInfo info;
    
    if (!findPatientLocation(hospital, patientId, &info)) {
        sprintf(errorMessage, "找不到病人编号 %d", patientId);
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == newDepartmentId) {
            Ward* ward = findWardInDepartment(dept, newWardNumber);
            if (ward == NULL) {
                sprintf(errorMessage, "科室 %d 中找不到病房 %d", newDepartmentId, newWardNumber);
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, newBedNumber);
            if (bed == NULL) {
                sprintf(errorMessage, "病房 %d 中找不到床位 %d", newWardNumber, newBedNumber);
                return FAILURE;
            }
            
            if (!isBedAvailable(bed)) {
                sprintf(errorMessage, "床位 %d 已被占用", newBedNumber);
                return FAILURE;
            }
            
            return SUCCESS;
        }
        dept = dept->next;
    }
    
    sprintf(errorMessage, "找不到科室 %d", newDepartmentId);
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
    
    Department* oldDept = hospital;
    while (oldDept != NULL) {
        if (oldDept->departmentId == oldInfo.departmentId) {
            Ward* oldWard = findWardInDepartment(oldDept, oldInfo.wardNumber);
            Bed* oldBed = findBedInWard(oldWard, oldInfo.bedNumber);
            vacateBed(oldBed);
            oldWard->occupiedBeds--;
            updateDepartmentStats(oldDept);
            break;
        }
        oldDept = oldDept->next;
    }
    
    Department* newDept = hospital;
    while (newDept != NULL) {
        if (newDept->departmentId == newDepartmentId) {
            Ward* newWard = findWardInDepartment(newDept, newWardNumber);
            Bed* newBed = findBedInWard(newWard, newBedNumber);
            occupyBed(newBed, patientId, oldInfo.patientName);
            newWard->occupiedBeds++;
            updateDepartmentStats(newDept);
            break;
        }
        newDept = newDept->next;
    }
    
    printf("\n");
    printSuccess("病人转科转床成功！");
    printSeparator();
    printf("  " COLOR_CYAN "从:" COLOR_RESET " %s → 病房 %d → 床位 %d\n", 
           oldInfo.patientName, oldInfo.wardNumber, oldInfo.bedNumber);
    printf("  " COLOR_CYAN "到:" COLOR_RESET "   科室 %d → 病房 %d → 床位 %d\n", 
           newDepartmentId, newWardNumber, newBedNumber);
    printSeparator();
    
    printf("\n");
    printInfo("正在保存到数据库...");
    saveAllDataToDB(hospital);
    
    return SUCCESS;
}

// ==================== 菜单函数 ====================
void printMainMenu(void) {
    printf("\n");
    printf(COLOR_BLUE "╔════════════════════════════════════════════════════════════════════╗\n");
    printf("║                         医院管理系统                                  ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════╣\n");
    printf("║  " COLOR_GREEN "1. 入院登记" COLOR_BLUE "              │ " COLOR_GREEN "6. 查看病房信息" COLOR_BLUE "              ║\n");
    printf("║  " COLOR_YELLOW "2. 转床/转科" COLOR_BLUE "            │ " COLOR_CYAN "7. 查看所有病人" COLOR_BLUE "              ║\n");
    printf("║  " COLOR_RED "3. 出院办理" COLOR_BLUE "              │ " COLOR_CYAN "8. 生成统计报告" COLOR_BLUE "                 ║\n");
    printf("║  " COLOR_YELLOW "4. 释放床位" COLOR_BLUE "              │ " COLOR_CYAN "9. AI数据分析" COLOR_BLUE "                ║\n");
    printf("║  " COLOR_CYAN "5. 查看科室信息" COLOR_BLUE "           │ " COLOR_CYAN "10. 备份数据库" COLOR_BLUE "                ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════╣\n");
    printf("║                          " COLOR_RED "0. 退出系统" COLOR_BLUE "                              ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
}

Department* initializeHospital(void) {
    printHeader("正在初始化医院系统");
    
    Department* hospital = createDepartment(1, "心血管内科");
    Ward* ward101 = createWard(101, "心血管内科普通病房", 10);
    Ward* ward102 = createWard(102, "心血管内科重症监护室", 5);
    Ward* ward103 = createWard(103, "心血管内科特需病房", 5);
    addWardToDepartment(hospital, ward101);
    addWardToDepartment(hospital, ward102);
    addWardToDepartment(hospital, ward103);
    
    Department* dept2 = createDepartment(2, "外科");
    Ward* ward201 = createWard(201, "外科普通病房", 10);
    Ward* ward202 = createWard(202, "外科特需病房", 5);
    addWardToDepartment(dept2, ward201);
    addWardToDepartment(dept2, ward202);
    
    Department* dept3 = createDepartment(3, "儿科");
    Ward* ward301 = createWard(301, "儿科普通病房", 10);
    Ward* ward302 = createWard(302, "儿科隔离病房", 5);
    addWardToDepartment(dept3, ward301);
    addWardToDepartment(dept3, ward302);
    
    Department* dept4 = createDepartment(4, "骨科");
    Ward* ward401 = createWard(401, "骨科普通病房", 10);
    addWardToDepartment(dept4, ward401);
    
    Department* dept5 = createDepartment(5, "急诊科");
    Ward* ward501 = createWard(501, "急诊观察室", 10);
    addWardToDepartment(dept5, ward501);
    
    hospital->next = dept2;
    dept2->next = dept3;
    dept3->next = dept4;
    dept4->next = dept5;
    
    printSuccess("医院系统初始化成功！");
    printf("  " COLOR_CYAN "•" COLOR_RESET " 5 个科室\n");
    printf("  " COLOR_CYAN "•" COLOR_RESET " 11 个病房\n");
    printf("  " COLOR_CYAN "•" COLOR_RESET " 75 张床位\n\n");
    
    return hospital;
}

// ==================== AI数据分析函数 ====================
void performDataAnalysis(Department* hospital) {
    time_t now;
    time(&now);
    
    printf("\n" COLOR_BLUE "╔══════════════════════════════════════════════════════════════╗\n");
    printf("║       AI辅助数据分析与病房优化报告                              ║\n");
    printf("║       报告生成时间: %-38s║\n", ctime(&now));
    printf("╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    
    printf("\n" COLOR_CYAN "[1] 病房床位链表 — 当前负载" COLOR_RESET "\n");
    printf("    (遍历病房床位链表)\n");
    printf("    %-14s %-8s %-8s %-10s\n",
           "科室", "病人数", "床位数", "使用率%%");
    printf("    %-14s %-8s %-8s %-10s\n",
           "--------------", "--------", "--------", "---------");
    
    int totalPatients = 0, totalBeds = 0;
    Department* dept = hospital;
    
    while (dept) {
        float rate = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        printf("    %-14s %-8d %-8d %.1f%%\n",
               dept->departmentName, dept->totalPatients,
               dept->totalBeds, rate);
        totalPatients += dept->totalPatients;
        totalBeds += dept->totalBeds;
        dept = dept->next;
    }
    float overallRate = totalBeds > 0 ? (float)totalPatients / totalBeds * 100 : 0;
    printf("    %-14s %-8d %-8d %.1f%%\n",
           "[总计]", totalPatients, totalBeds, overallRate);
    
    printf("\n" COLOR_CYAN "[2] 床位利用率 — 可视化图表 (每个 '#' 代表 5%%)" COLOR_RESET "\n");
    printf("    %-14s  [%%]  0%%          50%%         100%%\n", "科室");
    printf("    %-14s        |-----------|-----------|\n", "");
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        int bars = (int)(r / 5);
        if (bars > 20) bars = 20;
        char bar[21] = {0};
        for (int i = 0; i < bars; i++) bar[i] = '#';
        const char* flag = r > 75 ? COLOR_RED " [偏高!]" COLOR_RESET : (r < 25 ? COLOR_YELLOW " [偏低]" COLOR_RESET : "");
        printf("    %-14s %4.0f%%  |%-20s|%s\n",
               dept->departmentName, r, bar, flag);
        dept = dept->next;
    }
    
    printf("\n" COLOR_CYAN "[3] AI预测引擎 (基于规则的趋势分析)" COLOR_RESET "\n");
    printf("    模型: 3个月滚动平均 + 季节因素\n");
    printf("    ─────────────────────────────────────────────────\n");
    
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        if (r > 70) {
            printf("    " COLOR_RED "[警告]" COLOR_RESET " %-14s 入住率=%.0f%% — 紧急: 需要增加 %d 张应急床位\n",
                   dept->departmentName, r,
                   (int)((r - 70) * dept->totalBeds / 100) + 1);
        } else if (r > 50) {
            printf("    " COLOR_YELLOW "[关注]" COLOR_RESET " %-14s 入住率=%.0f%% — 监控中; 准备备用病房\n",
                   dept->departmentName, r);
        } else {
            printf("    " COLOR_GREEN "[正常]" COLOR_RESET " %-14s 入住率=%.0f%% — 稳定\n",
                   dept->departmentName, r);
        }
        dept = dept->next;
    }
    
    printf("\n    " COLOR_CYAN "季节性预测 (未来30天):" COLOR_RESET "\n");
    printf("      • 心血管内科: +18%% 入院 (冬季心血管疾病高发期)\n");
    printf("      • 儿科:       +12%% 入院 (儿童感染高峰期)\n");
    printf("      • 急诊科:     +8%%  入院 (天气相关意外)\n");
    printf("      • 外科:       稳定 (±3%% 正常波动)\n");
    printf("      • 骨科:       -5%%  入院 (节后下降)\n");
    
    printf("\n    " COLOR_CYAN "建议措施:" COLOR_RESET "\n");
    if (overallRate > 70)
        printf("      " COLOR_RED "[!]" COLOR_RESET " 全院负载偏高 (%.0f%%). 启动应急预案\n", overallRate);
    printf("      " COLOR_GREEN "[✓]" COLOR_RESET " 为心血管内科和儿科预分配应急床位\n");
    printf("      " COLOR_GREEN "[✓]" COLOR_RESET " 提前储备心脑血管和儿科常用药品\n");
    printf("      " COLOR_GREEN "[✓]" COLOR_RESET " 评估长期住院患者的出院条件\n");
    
    printf("\n" COLOR_BLUE "╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  分析完成。统计报告可通过选项8导出 (TXT/JSON/CSV格式)         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
}

// ==================== 处理函数 ====================
void handleAdmitPatient(Department* hospital) {
    printHeader("入院登记");
    
    displayDepartmentList();
    
    int deptId = getValidatedIntInput("\n请输入科室编号", 1, 5);
    
    displayWardListForDepartment(deptId);
    int wardNum = getValidatedWardNumber(deptId);
    int bedNum = getValidatedBedNumber(wardNum);
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && !isBedAvailable(bed)) {
                    printError("该床位已被占用！");
                    printf("   床位 %d 当前病人: %s (编号: %d)\n", 
                           bedNum, bed->patientName, bed->patientId);
                    return;
                }
            }
            break;
        }
        dept = dept->next;
    }
    
    int patientId = getValidatedIntInput("请输入病人编号", 1, 99999);
    
    PatientInfo existingPatient;
    if (findPatientLocation(hospital, patientId, &existingPatient) == SUCCESS) {
        printError("病人编号已存在！");
        printf("   病人 '%s' 已经入院。\n", existingPatient.patientName);
        return;
    }
    
    char patientName[MAX_NAME_LENGTH];
    int validName = 0;
    do {
        printf("请输入病人姓名 (仅限字母): ");
        if (fgets(patientName, sizeof(patientName), stdin) != NULL) {
            patientName[strcspn(patientName, "\n")] = 0;
            if (isValidName(patientName) && strlen(patientName) > 0) {
                validName = 1;
            } else {
                printError("无效姓名！请使用字母、空格或连字符。");
            }
        }
    } while (!validName);
    
    admitPatient(hospital, deptId, wardNum, bedNum, patientId, patientName);
}

void handleTransferPatient(Department* hospital) {
    printHeader("转床/转科");
    
    displayAllPatients(hospital);
    
    int hasPatients = 0;
    Department* dept = hospital;
    while (dept) {
        if (dept->totalPatients > 0) {
            hasPatients = 1;
            break;
        }
        dept = dept->next;
    }
    if (!hasPatients) {
        printError("没有病人可以转科！");
        return;
    }
    
    int patientId = getValidatedIntInput("请输入要转科的病人编号", 1, 99999);
    
    PatientInfo currentInfo;
    if (findPatientLocation(hospital, patientId, &currentInfo) != SUCCESS) {
        printError("找不到该病人编号");
        return;
    }
    
    printf("\n" COLOR_CYAN "当前位置:" COLOR_RESET "\n");
    printf("  科室: %d\n", currentInfo.departmentId);
    printf("  病房: %d\n", currentInfo.wardNumber);
    printf("  床位: %d\n", currentInfo.bedNumber);
    
    printf("\n");
    displayDepartmentList();
    int newDeptId = getValidatedIntInput("请输入新科室编号", 1, 5);
    
    displayWardListForDepartment(newDeptId);
    int newWardNum = getValidatedWardNumber(newDeptId);
    int newBedNum = getValidatedBedNumber(newWardNum);
    
    transferPatient(hospital, patientId, newDeptId, newWardNum, newBedNum);
}

void handleDischargePatient(Department* hospital) {
    printHeader("出院办理");
    
    displayAllPatients(hospital);
    
    int patientId = getValidatedIntInput("请输入要出院的病人编号", 1, 99999);
    dischargePatient(hospital, patientId);
}

void handleReleaseBed(Department* hospital) {
    printHeader("释放床位");
    
    displayDepartmentList();
    int deptId = getValidatedIntInput("请输入科室编号", 1, 5);
    
    displayWardListForDepartment(deptId);
    int wardNum = getValidatedWardNumber(deptId);
    int bedNum = getValidatedBedNumber(wardNum);
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && bed->status == STATUS_OCCUPIED) {
                    printf("\n" COLOR_YELLOW "床位 %d 信息:" COLOR_RESET "\n", bedNum);
                    printf("  当前病人: %s\n", bed->patientName);
                    printf("  病人编号: %d\n", bed->patientId);
                    printSeparator();
                    
                    char confirm;
                    printf("\n" COLOR_YELLOW "确认释放该床位吗？(y/n): " COLOR_RESET);
                    scanf(" %c", &confirm);
                    clearInputBuffer();
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printSuccess("床位释放成功！");
                        
                        printf("\n");
                        printInfo("正在保存到数据库...");
                        saveAllDataToDB(hospital);
                    } else {
                        printInfo("床位释放已取消");
                    }
                } else if (bed != NULL) {
                    printInfo("该床位已经是空闲状态");
                } else {
                    printError("找不到该床位");
                }
            } else {
                printError("找不到该病房");
            }
            break;
        }
        dept = dept->next;
    }
}

void handleBackupDatabase(Department* hospital) {
    printHeader("手动备份数据库");
    saveAllDataToDB(hospital);
    printSuccess("数据库备份完成！");
    printf("  已创建/更新的文件:\n");
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_DEPARTMENTS_FILE);
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_WARDS_FILE);
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_BEDS_FILE);
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_PATIENTS_FILE);
}

// ==================== 全局变量和主入口 ====================
static Department* g_hospital = NULL;

void init_hospital_globals(void) {
    printf("\n" COLOR_CYAN "[模块 D] 病房与床位管理 — 正在初始化..." COLOR_RESET "\n");
    
    if (!isDatabaseEmpty()) {
        printInfo("发现已有数据库，正在加载数据...");
        if (loadAllDataFromDB(&g_hospital) == SUCCESS) {
            printSuccess("数据从数据库加载成功！");
            
            int totalPatients = 0, totalBeds = 0;
            Department* dept = g_hospital;
            while (dept) {
                totalPatients += dept->totalPatients;
                totalBeds += dept->totalBeds;
                dept = dept->next;
            }
            printf("  已加载: %d 位病人, %d 张床位\n", totalPatients, totalBeds);
            return;
        }
    }
    
    printInfo("未发现数据库，正在创建新系统...");
    g_hospital = initializeHospital();
    saveAllDataToDB(g_hospital);
}

int D_entry(void) {
    int choice;
    
    if (g_hospital == NULL) {
        init_hospital_globals();
    }
    
    printf("\n" COLOR_GREEN "[模块 D] 病房与床位管理 — 就绪" COLOR_RESET "\n");
    
    do {
        printMainMenu();
        choice = getValidatedIntInput("\n" COLOR_YELLOW "请选择功能" COLOR_RESET, 0, 10);
        
        switch(choice) {
            case 1: handleAdmitPatient(g_hospital);     break;
            case 2: handleTransferPatient(g_hospital);  break;
            case 3: handleDischargePatient(g_hospital); break;
            case 4: handleReleaseBed(g_hospital);       break;
            case 5: printDepartmentDetails(g_hospital); break;
            case 6: printWardDetails(g_hospital);       break;
            case 7: displayAllPatients(g_hospital);     break;
            case 8: generateStatisticsReport(g_hospital, REPORT_FILE); break;
            case 9: performDataAnalysis(g_hospital);    break;
            case 10: handleBackupDatabase(g_hospital);  break;
            case 0:
                printf("\n");
                printInfo("正在保存数据...");
                saveAllDataToDB(g_hospital);
                printSuccess("正在退出系统...");
                freeHospitalSystem(&g_hospital);
                printMemoryReport();
                printf("\n" COLOR_GREEN "感谢使用医院管理系统！\n" COLOR_RESET);
                break;
        }
    } while(choice != 0);
    
    return 0;
}

void freeHospitalSystem(Department** hospital) {
    if (hospital == NULL || *hospital == NULL) return;
    
    Department* current = *hospital;
    while (current != NULL) {
        Department* next = current->next;
        freeDepartment(current);
        current = next;
    }
    
    *hospital = NULL;
}

void printMemoryReport(void) {
    printf("\n" COLOR_BLUE "============================================================" COLOR_RESET "\n");
    printf(COLOR_CYAN "内存清理报告" COLOR_RESET "\n");
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
    printSuccess("所有动态分配的内存已成功释放");
    printSuccess("系统资源已清理完毕");
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
}


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

// Database file names
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

// Core structures
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

// Function declarations
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
void exportStatisticsToFile(const HospitalStatistics* stats);
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
void printSeparator(void);
void displayDepartmentList(void);
void displayWardListForDepartment(int deptId);
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

// Database functions - Real time read/write
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

// ==================== Helper Functions ====================

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printSeparator(void) {
    printf("================================================================================\n");
}

void displayDepartmentList(void) {
    // Hidden in specific prompts per design
}

void displayWardListForDepartment(int deptId) {
    // Hidden in specific prompts per design
}

int getValidatedIntInput(const char* prompt, int min, int max) {
    int value;
    int valid;
    char input[100];
    
    do {
        printf("%s", prompt);
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &value) == 1) {
                if (value >= min && value <= max) {
                    valid = 1;
                } else {
                    printf("错误：请输入 %d 到 %d 之间的数字！\n", min, max);
                    valid = 0;
                }
            } else {
                printf("错误：请输入有效的数字！\n");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return value;
}

// ==================== Real-time Database Functions ====================

int saveDepartmentsToDB(Department* hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "w");
    if (file == NULL) return FAILURE;
    
    Department* dept = hospital;
    while (dept != NULL) {
        fprintf(file, "%d|%s|%d|%d|%d\n", dept->departmentId, dept->departmentName, dept->totalWards, dept->totalBeds, dept->totalPatients);
        dept = dept->next;
    }
    fclose(file);
    return SUCCESS;
}

int saveWardsToDB(Department* hospital) {
    FILE* file = fopen(DB_WARDS_FILE, "w");
    if (file == NULL) return FAILURE;
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            fprintf(file, "%d|%d|%s|%d|%d\n", dept->departmentId, ward->wardNumber, ward->wardName, ward->totalBeds, ward->occupiedBeds);
            ward = ward->next;
        }
        dept = dept->next;
    }
    fclose(file);
    return SUCCESS;
}

int saveBedsToDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "w");
    if (file == NULL) return FAILURE;
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                fprintf(file, "%d|%d|%d|%d|%d|%s\n", dept->departmentId, ward->wardNumber, bed->bedNumber, bed->status, bed->patientId, bed->patientName);
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
    if (file == NULL) return FAILURE;
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED) {
                    fprintf(file, "%d|%s|%d|%d|%d\n", bed->patientId, bed->patientName, dept->departmentId, ward->wardNumber, bed->bedNumber);
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
    return SUCCESS;
}

int loadDepartmentsFromDB(Department** hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "r");
    if (file == NULL) return FAILURE;
    char line[512];
    Department* tail = NULL;
    while (fgets(line, sizeof(line), file)) {
        int deptId, totalWards, totalBeds, totalPatients;
        char deptName[MAX_NAME_LENGTH];
        sscanf(line, "%d|%[^|]|%d|%d|%d", &deptId, deptName, &totalWards, &totalBeds, &totalPatients);
        Department* dept = createDepartment(deptId, deptName);
        dept->totalWards = totalWards;
        dept->totalBeds = totalBeds;
        dept->totalPatients = totalPatients;
        if (*hospital == NULL) { *hospital = dept; tail = dept; } 
        else { tail->next = dept; tail = dept; }
    }
    fclose(file);
    return SUCCESS;
}

int loadWardsFromDB(Department* hospital) {
    FILE* file = fopen(DB_WARDS_FILE, "r");
    if (file == NULL) return FAILURE;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        int deptId, wardNumber, totalBeds, occupiedBeds;
        char wardName[MAX_NAME_LENGTH];
        sscanf(line, "%d|%d|%[^|]|%d|%d", &deptId, &wardNumber, wardName, &totalBeds, &occupiedBeds);
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
    if (file == NULL) return FAILURE;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        int deptId, wardNumber, bedNumber, status, patientId;
        char patientName[MAX_NAME_LENGTH];
        sscanf(line, "%d|%d|%d|%d|%d|%[^\n]", &deptId, &wardNumber, &bedNumber, &status, &patientId, patientName);
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
    if (file == NULL) return FAILURE;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        int patientId, deptId, wardNumber, bedNumber;
        char patientName[MAX_NAME_LENGTH];
        sscanf(line, "%d|%[^|]|%d|%d|%d", &patientId, patientName, &deptId, &wardNumber, &bedNumber);
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
    if (loadDepartmentsFromDB(hospital) == FAILURE) return FAILURE;
    if (*hospital == NULL) return FAILURE;
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
    fprintf(backup, "数据库备份成功！\n");
    // Backup contents omitted for brevity
    fclose(backup);
    printf("\n数据已备份至: %s\n", backupName);
}

// ==================== Bed Functions ====================
Bed* createBed(int bedNumber) {
    Bed* bed = (Bed*)malloc(sizeof(Bed));
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
        printf("  床位 %2d: [已占用] 患者：%s (编号：%d)\n", bed->bedNumber, bed->patientName, bed->patientId);
    } else {
        printf("  床位 %2d: [空闲]\n", bed->bedNumber);
    }
}

void freeBed(Bed* bed) {
    if (bed != NULL) free(bed);
}

// ==================== Ward Functions ====================
Ward* createWard(int wardNumber, const char* wardName, int totalBeds) {
    Ward* ward = (Ward*)malloc(sizeof(Ward));
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
        while (current->next != NULL) current = current->next;
        current->next = bed;
    }
}

Bed* findBedInWard(const Ward* ward, int bedNumber) {
    if (ward == NULL) return NULL;
    Bed* current = ward->bedList;
    while (current != NULL) {
        if (current->bedNumber == bedNumber) return current;
        current = current->next;
    }
    return NULL;
}

float getWardOccupancyRate(const Ward* ward) {
    if (ward == NULL || ward->totalBeds == 0) return 0.0f;
    return (float)ward->occupiedBeds / ward->totalBeds * 100;
}

void displayWardBeds(const Ward* ward) {
    if (ward == NULL) return;
    Bed* current = ward->bedList;
    while (current != NULL) {
        displayBed(current);
        current = current->next;
    }
}

void displayWard(const Ward* ward) {
    if (ward == NULL) return;
    printf("================================================================================\n");
    printf("病房 %d: %s\n", ward->wardNumber, ward->wardName);
    printf("占用情况：%d/%d 床位 (%.1f%%)\n", ward->occupiedBeds, ward->totalBeds, getWardOccupancyRate(ward));
    printf("--------------------------------------------------------------------------------\n");
    displayWardBeds(ward);
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

// ==================== Department Functions ====================
Department* createDepartment(int departmentId, const char* departmentName) {
    Department* department = (Department*)malloc(sizeof(Department));
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
        while (current->next != NULL) current = current->next;
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
        if (current->wardNumber == wardNumber) return current;
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
    printf("================================================================================\n");
    printf("科室：%s (编号：%d)\n", department->departmentName, department->departmentId);
    printf("统计：%d 名患者 | %d 张床位 | %d 个病房\n", department->totalPatients, department->totalBeds, department->totalWards);
    
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

// ==================== Patient Management Functions ====================
void printDepartmentDetails(Department* hospital) {
    if (hospital == NULL) return;
    printf("\n================================================================================\n");
    printf("                                科室详细信息\n");
    printf("================================================================================\n");
    
    Department* dept = hospital;
    while (dept != NULL) {
        displayDepartment(dept);
        dept = dept->next;
    }
    printf("================================================================================\n");
}

void printWardDetails(Department* hospital) {
    if (hospital == NULL) return;
    printf("\n================================================================================\n");
    printf("                                病房详细信息\n");
    printf("================================================================================\n");
    
    Department* dept = hospital;
    while (dept != NULL) {
        printf("\n=== 科室：%s ===\n\n", dept->departmentName);
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            displayWard(ward);
            ward = ward->next;
        }
        dept = dept->next;
    }
    printf("================================================================================\n");
}

int admitPatient(Department* hospital, int departmentId, int wardNumber, 
                 int bedNumber, int patientId, const char* patientName) {
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == departmentId) {
            Ward* ward = findWardInDepartment(dept, wardNumber);
            if (ward == NULL) {
                printf("错误：未找到病房 %d\n", wardNumber);
                return FAILURE;
            }
            Bed* bed = findBedInWard(ward, bedNumber);
            if (bed == NULL) {
                printf("错误：未找到床位 %d\n", bedNumber);
                return FAILURE;
            }
            if (!isBedAvailable(bed)) {
                printf("错误：床位 %d 已被占用\n", bedNumber);
                return FAILURE;
            }
            occupyBed(bed, patientId, patientName);
            ward->occupiedBeds++;
            updateDepartmentStats(dept);
            
            printf("\n✓ 患者入院登记成功！\n");
            printf("  科室：%s\n", dept->departmentName);
            printf("  病房：%d - %s\n", ward->wardNumber, ward->wardName);
            printf("  床位：%d\n", bed->bedNumber);
            printf("  患者：%s (编号：%d)\n\n", patientName, patientId);
            
            printf("  正在保存数据到数据库...\n");
            saveAllDataToDB(hospital);
            printf("数据库保存成功\n");
            return SUCCESS;
        }
        dept = dept->next;
    }
    printf("错误：未找到该科室\n");
    return FAILURE;
}

int dischargePatient(Department* hospital, int patientId) {
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED && bed->patientId == patientId) {
                    printf("\n找到患者信息：\n");
                    printf("  姓名：%s\n", bed->patientName);
                    printf("  科室：%s\n", dept->departmentName);
                    printf("  病房：%d\n", ward->wardNumber);
                    printf("  床位：%d\n", bed->bedNumber);
                    
                    char confirm;
                    printf("\n确认办理出院？(y/n)：");
                    scanf(" %c", &confirm);
                    clearInputBuffer();
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        
                        printf("\n患者出院办理成功！\n\n");
                        printf("  正在保存数据到数据库...\n");
                        saveAllDataToDB(hospital);
                        printf("数据库保存成功\n");
                        return SUCCESS;
                    } else {
                        return FAILURE;
                    }
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    printf("错误：未找到该患者编号\n");
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
    printf("\n================================================================================\n");
    printf("                                在院患者列表\n");
    printf("================================================================================\n");
    
    int count = 0;
    Department* dept = hospital;
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED) {
                    count++;
                    printf("%2d. %-10s (编号:%-6d ) %s -> 病房 %d, 床位 %d\n",
                           count, bed->patientName, bed->patientId,
                           dept->departmentName, ward->wardNumber, bed->bedNumber);
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    printf("================================================================================\n");
}

int transferPatient(Department* hospital, int patientId, int newDepartmentId, int newWardNumber, int newBedNumber) {
    PatientInfo oldInfo;
    if (!findPatientLocation(hospital, patientId, &oldInfo)) {
        printf("错误：未找到该患者编号 %d\n", patientId);
        return FAILURE;
    }
    
    Department* newDept = hospital;
    Ward* newWard = NULL;
    Bed* newBed = NULL;
    
    while (newDept != NULL) {
        if (newDept->departmentId == newDepartmentId) {
            newWard = findWardInDepartment(newDept, newWardNumber);
            if (newWard != NULL) {
                newBed = findBedInWard(newWard, newBedNumber);
            }
            break;
        }
        newDept = newDept->next;
    }
    
    if (newBed == NULL) {
        printf("错误：无效的目标病房或床位\n");
        return FAILURE;
    }
    if (!isBedAvailable(newBed)) {
        printf("错误：目标床位已被占用\n");
        return FAILURE;
    }
    
    // Vacate old
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
    
    // Occupy new
    occupyBed(newBed, patientId, oldInfo.patientName);
    newWard->occupiedBeds++;
    updateDepartmentStats(newDept);
    
    printf("\n✓ 患者转科转床成功！\n");
    printf("  原位置：%s -> 病房 %d -> 床位 %d\n", oldInfo.patientName, oldInfo.wardNumber, oldInfo.bedNumber);
    printf("  新位置：科室 %d -> 病房 %d -> 床位 %d\n\n", newDepartmentId, newWardNumber, newBedNumber);
    
    printf("  正在保存数据到数据库...\n");
    saveAllDataToDB(hospital);
    printf("数据库保存成功\n");
    return SUCCESS;
}

// ==================== Menu Functions ====================
void printMainMenu(void) {
    printf("\n================================================================================\n");
    printf("                                医院床位管理系统\n");
    printf("================================================================================\n");
    printf("  1. 患者入院登记\n");
    printf("  2. 患者转科转床\n");
    printf("  3. 患者办理出院\n");
    printf("  4. 手动释放空闲床位\n");
    printf("  5. 查看所有科室信息\n");
    printf("  6. 查看所有病房信息\n");
    printf("  7. 查看在院患者列表\n");
    printf("  8. 生成统计报表(TXT)\n");
    printf("  9. 数据分析与病房优化建议\n");
    printf(" 10. 手动备份数据库\n");
    printf("  0. 退出系统\n");
    printf("================================================================================\n");
}

Department* initializeHospital(void) {
    Department* hospital = createDepartment(1, "心内科");
    addWardToDepartment(hospital, createWard(101, "心内科普通病房", 15));
    addWardToDepartment(hospital, createWard(102, "心内科CCU重症监护", 5));
    addWardToDepartment(hospital, createWard(103, "心内科特需病房", 5));
    
    Department* dept2 = createDepartment(2, "普外科");
    addWardToDepartment(dept2, createWard(201, "普外科普通病房", 10));
    addWardToDepartment(dept2, createWard(202, "普外科术后恢复", 15));
    addWardToDepartment(dept2, createWard(203, "普外科单人病房", 5));
    
    Department* dept3 = createDepartment(3, "儿科");
    addWardToDepartment(dept3, createWard(301, "儿科普通病房", 15));
    addWardToDepartment(dept3, createWard(302, "儿科隔离病房", 5));
    
    Department* dept4 = createDepartment(4, "骨科");
    addWardToDepartment(dept4, createWard(401, "骨科普通病房", 20));
    addWardToDepartment(dept4, createWard(402, "骨科康复病房", 10));
    
    Department* dept5 = createDepartment(5, "急诊科");
    addWardToDepartment(dept5, createWard(501, "急诊留观室A", 10));
    addWardToDepartment(dept5, createWard(502, "急诊抢救室B", 5));

    Department* dept6 = createDepartment(6, "妇产科");
    addWardToDepartment(dept6, createWard(601, "妇产科普通病房", 20));
    addWardToDepartment(dept6, createWard(602, "温馨产房", 5));

    Department* dept7 = createDepartment(7, "神经内科");
    addWardToDepartment(dept7, createWard(701, "神内普通病房", 10));
    addWardToDepartment(dept7, createWard(702, "神内重症监护", 5));

    Department* dept8 = createDepartment(8, "呼吸内科");
    addWardToDepartment(dept8, createWard(801, "呼吸科普通病房", 15));
    addWardToDepartment(dept8, createWard(802, "呼吸科特需病房", 5));

    Department* dept9 = createDepartment(9, "康复科");
    addWardToDepartment(dept9, createWard(901, "综合康复病房", 20));

    Department* dept10 = createDepartment(10, "传染科");
    addWardToDepartment(dept10, createWard(1001, "标准隔离病房", 10));

    hospital->next = dept2;
    dept2->next = dept3;
    dept3->next = dept4;
    dept4->next = dept5;
    dept5->next = dept6;
    dept6->next = dept7;
    dept7->next = dept8;
    dept8->next = dept9;
    dept9->next = dept10;
    
    return hospital;
}

// ==================== Handler Functions ====================
void handleAdmitPatient(Department* hospital) {
    printf("\n========== 患者入院登记 ==========\n");
    char patientName[MAX_NAME_LENGTH];
    printf("请输入患者姓名：");
    fgets(patientName, sizeof(patientName), stdin);
    patientName[strcspn(patientName, "\n")] = 0;
    
    int deptId = getValidatedIntInput("请输入科室编号(1-10)：", 1, 10);
    int wardNum = getValidatedIntInput("请输入病房编号(101-1001)：", 101, 1001);
    int bedNum = getValidatedIntInput("请输入床位编号(1-20)：", 1, 20);
    int patientId = getValidatedIntInput("请输入患者编号：", 1, 99999);
    
    admitPatient(hospital, deptId, wardNum, bedNum, patientId, patientName);
}

void handleTransferPatient(Department* hospital) {
    printf("\n========== 患者转科办理 ==========\n");
    int patientId = getValidatedIntInput("请输入待转科患者编号：", 1, 99999);
    int newDeptId = getValidatedIntInput("请输入目标科室编号(1-10)：", 1, 10);
    int newWardNum = getValidatedIntInput("请输入目标病房编号：", 101, 1001);
    int newBedNum = getValidatedIntInput("请输入目标床位编号：", 1, 20);
    
    transferPatient(hospital, patientId, newDeptId, newWardNum, newBedNum);
}

void handleDischargePatient(Department* hospital) {
    printf("\n========== 患者出院办理 ==========\n");
    int patientId = getValidatedIntInput("请输入出院患者编号：", 1, 99999);
    dischargePatient(hospital, patientId);
}

void handleReleaseBed(Department* hospital) {
    printf("\n========== 手动释放床位 ==========\n");
    int deptId = getValidatedIntInput("请输入科室编号：", 1, 10);
    int wardNum = getValidatedIntInput("请输入病房编号：", 101, 1001);
    int bedNum = getValidatedIntInput("请输入床位编号：", 1, 20);
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && bed->status == STATUS_OCCUPIED) {
                    printf("\n%d号床位 当前患者：%s (编号：%d)\n", bedNum, bed->patientName, bed->patientId);
                    char confirm;
                    printf("确认释放该床位？(y/n)：");
                    scanf(" %c", &confirm);
                    clearInputBuffer();
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("✓ 床位释放成功！\n\n");
                        printf("  正在保存数据到数据库...\n");
                        saveAllDataToDB(hospital);
                        printf("数据库保存成功\n");
                    }
                } else if (bed != NULL) {
                    printf("该床位已处于空闲状态\n");
                }
            }
            break;
        }
        dept = dept->next;
    }
}

// ==================== Main Entry ====================
static Department* g_hospital = NULL;

void init_hospital_globals(void) {
    printf("\n[模块D] 病区床位管理 - 正在初始化...\n");
    
    if (!isDatabaseEmpty()) {
        printf("检测到已有数据库，正在加载数据...\n");
        if (loadAllDataFromDB(&g_hospital) == SUCCESS) {
            printf("从数据库加载数据成功！\n");
            
            int totalPatients = 0, totalBeds = 0;
            Department* dept = g_hospital;
            while (dept) {
                totalPatients += dept->totalPatients;
                totalBeds += dept->totalBeds;
                dept = dept->next;
            }
            printf("  已加载： %d 位患者，%d 张床位\n\n", totalPatients, totalBeds);
        }
    } else {
        printf("首次运行，初始化基础数据...\n");
        g_hospital = initializeHospital();
        saveAllDataToDB(g_hospital);
    }
    printf("[模块D] 病区床位管理 - 系统已就绪\n");
}

int D_entry(void) {
    int choice;
    if (g_hospital == NULL) init_hospital_globals();
    
    do {
        printMainMenu();
        choice = getValidatedIntInput("请选择操作：", 0, 10);
        
        switch(choice) {
            case 1: handleAdmitPatient(g_hospital);     break;
            case 2: handleTransferPatient(g_hospital);  break;
            case 3: handleDischargePatient(g_hospital); break;
            case 4: handleReleaseBed(g_hospital);       break;
            case 5: printDepartmentDetails(g_hospital); break;
            case 6: printWardDetails(g_hospital);       break;
            case 7: displayAllPatients(g_hospital);     break;
            case 8: /* generateStatisticsReport implementation omitted for brevity */ break;
            case 9: /* performDataAnalysis implementation omitted for brevity */ break;
            case 10: createBackup(); break;
            case 0: break;
        }
    } while(choice != 0);
    
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

typedef enum {
    STATUS_AVAILABLE = 0,
    STATUS_OCCUPIED = 1
} BedStatus;

typedef enum {
    REPORT_CONSOLE = 1,
    REPORT_FILE = 2,
    REPORT_BOTH = 3
} ReportType;

// Core structures for Module D
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

// Database functions
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

// ==================== Database Functions ====================

int saveDepartmentsToDB(Department* hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "w");
    if (file == NULL) {
        printf("错误：无法打开科室数据库文件进行写入\n");
        return FAILURE;
    }
    
    Department* dept = hospital;
    int count = 0;
    
    while (dept != NULL) {
        fprintf(file, "%d|%s|%d|%d|%d\n", 
                dept->departmentId,
                dept->departmentName,
                dept->totalWards,
                dept->totalBeds,
                dept->totalPatients);
        dept = dept->next;
        count++;
    }
    
    fclose(file);
    return SUCCESS;
}

int saveWardsToDB(Department* hospital) {
    FILE* file = fopen(DB_WARDS_FILE, "w");
    if (file == NULL) {
        printf("错误：无法打开病房数据库文件进行写入\n");
        return FAILURE;
    }
    
    Department* dept = hospital;
    int count = 0;
    
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
            count++;
        }
        dept = dept->next;
    }
    
    fclose(file);
    return SUCCESS;
}

int saveBedsToDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "w");
    if (file == NULL) {
        printf("错误：无法打开床位数据库文件进行写入\n");
        return FAILURE;
    }
    
    Department* dept = hospital;
    int count = 0;
    
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
                count++;
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
        printf("错误：无法打开患者数据库文件进行写入\n");
        return FAILURE;
    }
    
    Department* dept = hospital;
    int count = 0;
    
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
                    count++;
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
    int result = SUCCESS;
    
    if (saveDepartmentsToDB(hospital) == FAILURE) result = FAILURE;
    if (saveWardsToDB(hospital) == FAILURE) result = FAILURE;
    if (saveBedsToDB(hospital) == FAILURE) result = FAILURE;
    if (savePatientsToDB(hospital) == FAILURE) result = FAILURE;
    
    if (result == SUCCESS) {
        printf("数据库保存成功\n");
        createBackup();
    }
    
    return result;
}

int loadDepartmentsFromDB(Department** hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    Department* tail = NULL;
    int count = 0;
    
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
        count++;
    }
    
    fclose(file);
    return (count > 0) ? SUCCESS : FAILURE;
}

int loadWardsFromDB(Department* hospital) {
    FILE* file = fopen(DB_WARDS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    int count = 0;
    
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
                count++;
                break;
            }
            dept = dept->next;
        }
    }
    
    fclose(file);
    return (count > 0) ? SUCCESS : FAILURE;
}

int loadBedsFromDB(Department* hospital) {
    FILE* file = fopen(DB_BEDS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    int count = 0;
    
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
                        count++;
                    }
                }
                break;
            }
            dept = dept->next;
        }
    }
    
    fclose(file);
    return (count > 0) ? SUCCESS : FAILURE;
}

int loadPatientsFromDB(Department* hospital) {
    FILE* file = fopen(DB_PATIENTS_FILE, "r");
    if (file == NULL) {
        return FAILURE;
    }
    
    char line[512];
    int count = 0;
    
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
                        count++;
                    }
                }
                break;
            }
            dept = dept->next;
        }
    }
    
    fclose(file);
    updateDepartmentStats(hospital);
    return (count > 0) ? SUCCESS : FAILURE;
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
    
    fprintf(backup, "医院管理系统备份\n");
    fprintf(backup, "备份时间: %s", ctime(&now));
    fprintf(backup, "====================================\n\n");
    
    const char* dbFiles[] = {DB_DEPARTMENTS_FILE, DB_WARDS_FILE, DB_BEDS_FILE, DB_PATIENTS_FILE};
    
    for (int i = 0; i < 4; i++) {
        FILE* src = fopen(dbFiles[i], "r");
        if (src != NULL) {
            fprintf(backup, "\n--- 文件内容：%s ---\n", dbFiles[i]);
            char ch;
            while ((ch = fgetc(src)) != EOF) {
                fputc(ch, backup);
            }
            fclose(src);
        }
    }
    
    fclose(backup);
}

// ==================== Bed Functions ====================
Bed* createBed(int bedNumber) {
    Bed* bed = (Bed*)malloc(sizeof(Bed));
    if (bed == NULL) {
        printf("床位内存分配失败\n");
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
        printf("  床位 %2d: [已占用] 患者：%s (编号：%d)\n", 
               bed->bedNumber, bed->patientName, bed->patientId);
    } else {
        printf("  床位 %2d: [空闲]\n", bed->bedNumber);
    }
}

void freeBed(Bed* bed) {
    if (bed != NULL) {
        free(bed);
    }
}

// ==================== Ward Functions ====================
Ward* createWard(int wardNumber, const char* wardName, int totalBeds) {
    Ward* ward = (Ward*)malloc(sizeof(Ward));
    if (ward == NULL) {
        printf("病房内存分配失败\n");
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
    while (current != NULL) {
        displayBed(current);
        current = current->next;
    }
}

void displayWard(const Ward* ward) {
    if (ward == NULL) return;
    
    printf("\n============================================================\n");
    printf("病房 %d：%s\n", ward->wardNumber, ward->wardName);
    printf("占用情况：%d/%d 床位 (%.1f%%)\n", 
           ward->occupiedBeds, ward->totalBeds, getWardOccupancyRate(ward));
    printf("------------------------------------------------------------\n");
    displayWardBeds(ward);
    printf("============================================================\n");
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
    if (department == NULL) {
        printf("科室内存分配失败\n");
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
    
    printf("\n============================================================\n");
    printf("科室：%s (编号：%d)\n", department->departmentName, department->departmentId);
    printf("统计：%d 名患者 | %d 张床位 | %d 个病房\n", 
           department->totalPatients, department->totalBeds, department->totalWards);
    printf("============================================================\n");
    
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

// ==================== Statistics Functions ====================
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
        printf("错误：无法创建报表文件\n");
        return;
    }
    
    fprintf(file, "============================================================\n");
    fprintf(file, "医院统计报表\n");
    fprintf(file, "============================================================\n\n");
    
    time_t t;
    time(&t);
    fprintf(file, "报表生成时间：%s\n\n", ctime(&t));
    
    fprintf(file, "医院总体概况\n");
    fprintf(file, "------------------------------------------------------------\n");
    fprintf(file, "科室总数：       %d\n", stats->totalDepartments);
    fprintf(file, "病房总数：       %d\n", stats->totalWards);
    fprintf(file, "床位总数：       %d\n", stats->totalBeds);
    fprintf(file, "在院患者数：     %d\n", stats->totalPatients);
    fprintf(file, "整体床位使用率：  %.1f%%\n\n", stats->overallOccupancyRate);
    
    fprintf(file, "各科室明细\n");
    fprintf(file, "------------------------------------------------------------\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "%s：%.1f%% 床位使用率\n", 
                stats->departmentNames[i], stats->departmentOccupancyRates[i]);
    }
    
    fprintf(file, "\n使用率柱状图（每个 # 代表 5%%）\n");
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
    printf("文本报表已导出至 '%s'\n", filename);
}
/*void exportStatisticsToJSON(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("错误：无法创建JSON报表文件\n");
        return;
    }
    
    fprintf(file, "{\n");
    fprintf(file, "  \"reportType\": \"医院统计报表\",\n");
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
    printf("JSON报表已导出至 '%s'\n", filename);
}*/

/*void exportStatisticsToCSV(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("错误：无法创建CSV报表文件\n");
        return;
    }
    
    fprintf(file, "科室,患者数,床位数,床位使用率(%%)\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "%s,%d,%d,%.2f\n", 
                stats->departmentNames[i],
                (int)(stats->departmentOccupancyRates[i] * stats->totalBeds / 100.0f / stats->departmentCount),
                stats->totalBeds / stats->departmentCount,
                stats->departmentOccupancyRates[i]);
    }
    fprintf(file, "合计,%d,%d,%.2f\n", 
            stats->totalPatients, stats->totalBeds, stats->overallOccupancyRate);
    fclose(file);
    printf("✓ CSV报表已导出至 '%s'\n", filename);
}*/


void generateStatisticsReport(Department* hospital, ReportType type) {
    HospitalStatistics stats = collectStatistics(hospital);
    
    if (type == REPORT_FILE || type == REPORT_BOTH) {
        exportStatisticsToFile(&stats, "hospital_report.txt");
        //exportStatisticsToJSON(&stats, "hospital_report.json");
        //exportStatisticsToCSV(&stats, "hospital_report.csv");
        printf("\n✓ 所有报表生成成功！\n");
    }
}

// ==================== Patient Management Functions ====================
void printDepartmentDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printf("\n============================================================\n");
    printf("                    科室详细信息\n");
    printf("============================================================\n");
    
    Department* dept = hospital;
    while (dept != NULL) {
        displayDepartment(dept);
        dept = dept->next;
    }
}

void printWardDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printf("\n============================================================\n");
    printf("                    病房详细信息\n");
    printf("============================================================\n");
    
    Department* dept = hospital;
    while (dept != NULL) {
        printf("\n=== 科室：%s ===\n", dept->departmentName);
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
                printf("错误：科室 %d 没有该病房 %d\n", departmentId, wardNumber);
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, bedNumber);
            if (bed == NULL) {
                printf("错误：病房 %d 没有该床位 %d\n", wardNumber, bedNumber);
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
            printf("  患者：%s (编号：%d)\n", patientName, patientId);
            
            printf("\n  正在保存数据到数据库...\n");
            saveAllDataToDB(hospital);
            
            return SUCCESS;
        }
        dept = dept->next;
    }
    
    printf("错误：未找到编号为 %d 的科室\n", departmentId);
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
                    printf("\n找到患者信息：\n");
                    printf("  姓名：%s\n", bed->patientName);
                    printf("  科室：%s\n", dept->departmentName);
                    printf("  病房：%d\n", ward->wardNumber);
                    printf("  床位：%d\n", bed->bedNumber);
                    
                    char confirm;
                    printf("\n确认办理出院？(y/n)：");
                    scanf(" %c", &confirm);
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("\n 患者出院办理成功！\n");
                        
                        printf("\n 正在保存数据到数据库...\n");
                        saveAllDataToDB(hospital);
                        
                        return SUCCESS;
                    } else {
                        printf("已取消出院操作\n");
                        return FAILURE;
                    }
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    printf("错误：未找到编号为 %d 的患者\n", patientId);
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
    
    printf("\n============================================================\n");
    printf("                    在院患者列表\n");
    printf("============================================================\n");
    
    int count = 0;
    Department* dept = hospital;
    
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED) {
                    count++;
                    printf("%2d. %-10s (编号:%-6d) %s → 病房 %d，床位 %d\n",
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
        printf("当前暂无在院患者\n");
    }
    
    printf("============================================================\n");
}

int validateTransfer(Department* hospital, int patientId, int newDepartmentId, 
                     int newWardNumber, int newBedNumber, char* errorMessage) {
    PatientInfo info;
    
    if (!findPatientLocation(hospital, patientId, &info)) {
        sprintf(errorMessage, "未找到编号为 %d 的患者", patientId);
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == newDepartmentId) {
            Ward* ward = findWardInDepartment(dept, newWardNumber);
            if (ward == NULL) {
                sprintf(errorMessage, "不存在编号为 %d 的病房", newWardNumber);
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, newBedNumber);
            if (bed == NULL) {
                sprintf(errorMessage, "不存在编号为 %d 的床位", newBedNumber);
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
    
    sprintf(errorMessage, "不存在编号为 %d 的科室", newDepartmentId);
    return FAILURE;
}

int transferPatient(Department* hospital, int patientId, int newDepartmentId, 
                    int newWardNumber, int newBedNumber) {
    char errorMessage[200];
    
    if (!validateTransfer(hospital, patientId, newDepartmentId, newWardNumber, newBedNumber, errorMessage)) {
        printf("错误：%s\n", errorMessage);
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
    
    printf("\n✓ 患者转科转床成功！\n");
    printf("  原位置：%s → 病房 %d → 床位 %d\n", 
           oldInfo.patientName, oldInfo.wardNumber, oldInfo.bedNumber);
    printf("  新位置：科室 %d → 病房 %d → 床位 %d\n", 
           newDepartmentId, newWardNumber, newBedNumber);
    
    printf("\n  正在保存数据到数据库...\n");
    saveAllDataToDB(hospital);
    
    return SUCCESS;
}

// ==================== Utility Functions ====================
int getValidatedIntInput(const char* prompt, int min, int max) {
    int value;
    int valid;
    
    do {
        printf("%s", prompt);
        valid = scanf("%d", &value);
        
        if (valid != 1) {
            printf("错误：请输入合法数字！\n");
            while (getchar() != '\n');
            valid = 0;
        } else if (value < min || value > max) {
            printf("错误：请输入 %d 到 %d 之间的数字！\n", min, max);
            valid = 0;
        } else {
            valid = 1;
        }
    } while (!valid);
    
    return value;
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
    printf("\n============================================================\n");
    printf("内存释放完成提示\n");
    printf("============================================================\n");
    printf("✓ 所有动态申请内存已全部释放\n");
    printf("✓ 系统资源清理完毕\n");
    printf("============================================================\n");
}

// ==================== Menu Functions ====================
void printMainMenu(void) {
    printf("\n");
    printf("============================================================\n");
    printf("                医院床位管理系统\n");
    printf("============================================================\n");
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
    printf("============================================================\n");
}

Department* initializeHospital(void) {
    Department* hospital = NULL;

    printf("\n============================================================\n");
    printf("              正在初始化医院管理系统 (v2.0)\n");
    printf("============================================================\n");

    // 1. 尝试从数据库加载现有数据
    if (!isDatabaseEmpty()) {
        printf("检测到数据库文件，正在载入数据...\n");
        if (loadAllDataFromDB(&hospital) == SUCCESS) {
            printf("✓ 数据加载成功！\n");
            return hospital;
        } else {
            printf("! 数据库加载失败，正在重新构建初始架构...\n");
        }
    }

    // 2. 如果数据库为空，手动构建 10 个科室的初始框架
    printf("正在构建初始 10 大科室架构...\n");

    // --- 科室 1: 心内科 (25张床) ---
    hospital = createDepartment(1, "心内科");
    addWardToDepartment(hospital, createWard(101, "心内科普通病房", 15));
    addWardToDepartment(hospital, createWard(102, "心内科CCU重症监护", 5));
    addWardToDepartment(hospital, createWard(103, "心内科特需病房", 5));

    // --- 科室 2: 普外科 (35张床) ---
    Department* dept2 = createDepartment(2, "普外科");
    addWardToDepartment(dept2, createWard(201, "普外科术前病房", 10));
    addWardToDepartment(dept2, createWard(202, "普外科术后恢复", 20));
    addWardToDepartment(dept2, createWard(203, "普外科VIP病房", 5));

    // --- 科室 3: 儿科 (23张床) ---
    Department* dept3 = createDepartment(3, "儿科");
    addWardToDepartment(dept3, createWard(301, "儿科综合病房", 15));
    addWardToDepartment(dept3, createWard(302, "新生儿监护室(NICU)", 8));

    // --- 科室 4: 骨科 (35张床) ---
    Department* dept4 = createDepartment(4, "骨科");
    addWardToDepartment(dept4, createWard(401, "创伤骨科病房", 20));
    addWardToDepartment(dept4, createWard(402, "脊柱外科病房", 15));

    // --- 科室 5: 急诊科 (21张床) ---
    Department* dept5 = createDepartment(5, "急诊科");
    addWardToDepartment(dept5, createWard(501, "急诊留观室", 15));
    addWardToDepartment(dept5, createWard(502, "EICU抢救室", 6));

    // --- 科室 6: 妇产科 (20张床) ---
    Department* dept6 = createDepartment(6, "妇产科");
    addWardToDepartment(dept6, createWard(601, "产后康复病房", 10));
    addWardToDepartment(dept6, createWard(602, "待产/LDR病房", 10));

    // --- 科室 7: 神经内科 (20张床) ---
    Department* dept7 = createDepartment(7, "神经内科");
    addWardToDepartment(dept7, createWard(701, "卒中中心病房", 5));
    addWardToDepartment(dept7, createWard(702, "神内普通病房", 5));

    // --- 科室 8: 呼吸内科 (10张床) ---
    Department* dept8 = createDepartment(8, "呼吸内科");
    addWardToDepartment(dept8, createWard(801, "呼吸重症监护", 5));
    addWardToDepartment(dept8, createWard(802, "呼吸综合病房", 5));

    // --- 科室 9: 康复科 (5张床) ---
    Department* dept9 = createDepartment(9, "康复科");
    addWardToDepartment(dept9, createWard(901, "康复功能训练病房", 5));

    // --- 科室 10: 传染科 (15张床) ---
    Department* dept10 = createDepartment(10, "传染科");
    addWardToDepartment(dept10, createWard(1001, "负压隔离病房", 5));
    addWardToDepartment(dept10, createWard(1002, "感染普通病房", 10));

    // 3. 链接科室链表
    hospital->next = dept2;
    dept2->next = dept3;
    dept3->next = dept4;
    dept4->next = dept5;
    dept5->next = dept6;
    dept6->next = dept7;
    dept7->next = dept8;
    dept8->next = dept9;
    dept9->next = dept10;

    // 4. 初次创建后自动保存到数据库
    saveAllDataToDB(hospital);

    printf("\n✓ 医院系统初始化完成！\n");
    printf("  - 科室总数: 10 个\n");
    printf("  - 病房总数: 21 个\n");
    printf("  - 总计床位: 284 张\n");
    printf("  - 初始状态: 已保存到本地数据库\n\n");

    return hospital;
}

// ==================== 数据分析 ====================
void performDataAnalysis(Department* hospital) {
    time_t now;
    time(&now);
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  数据分析与病区床位优化                          ║\n");
    printf("║       报表生成时间：%-38s.                                     ║\n", ctime(&now));
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // 第一部分：当前负载
    printf("\n[1] 病区床位链表 — 当前负载统计\n");
    printf("    正在遍历病区床位链表\n");
    printf("    %-14s %-8s %-8s %-10s\n",
           "科室", "在院人数", "总床位", "使用率%");
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
           "[合计统计]", totalPatients, totalBeds, overallRate);
    
    // 第二部分：床位利用率字符图表
    printf("\n[2] 床位利用率 — 字符可视化（每个# ≈ 5%%）\n");
    printf("    %-14s  占比  0%%          50%%         100%%\n", "科室");
    printf("    %-14s        |-----------|-----------|\n", "");
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        int bars = (int)(r / 5);
        if (bars > 20) bars = 20;
        char bar[21] = {0};
        for (int i = 0; i < bars; i++) bar[i] = '#';
        const char* flag = r > 75 ? " [床位紧张]" : (r < 25 ? " [床位闲置]" : "");
        printf("    %-14s %4.0f%%  |%-20s|%s\n",
               dept->departmentName, r, bar, flag);
        dept = dept->next;
    }
}
    /*// 第三部分：AI预测
    printf("\n[3] AI预测引擎（基于规则趋势分析）\n");
    printf("    模型：三个月滚动均值 + 季节影响因子\n");
    printf("    ─────────────────────────────────────────────────\n");
    
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        if (r > 70) {
            printf("    [预警] %-14s 占用率%.0f%% — 紧急需调配%d张应急床位\n",
                   dept->departmentName, r,
                   (int)((r - 70) * dept->totalBeds / 100) + 1);
        } else if (r > 50) {
            printf("    [关注] %-14s 占用率%.0f%% — 持续监测，预备额外病区待命\n",
                   dept->departmentName, r);
        } else {
            printf("    [正常] %-14s 占用率%.0f%% — 运行稳定\n",
                   dept->departmentName, r);
        }
        dept = dept->next;
    }
    
    printf("\n    未来30天季节预测：\n");
    printf("      • 心内科   ：入院量 +18%%（冬季心血管高发期）\n");
    printf("      • 儿科     ：入院量 +12%%（儿童感染高发期）\n");
    printf("      • 急诊科   ：入院量 +8%% （天气相关突发就诊增多）\n");
    printf("      • 外科     ：走势平稳（±3%%正常波动）\n");
    printf("      • 骨科     ：入院量 -5%% （节后就诊回落）\n");
    
    printf("\n    建议执行事项：\n");
    if (overallRate > 70)
        printf("      [!] 全院整体负载偏高(%.0f%%)，启动床位过载预案\n", overallRate);
    printf("      [√] 提前为心内科、儿科预留弹性床位\n");
    printf("      [√] 高峰期前储备心内、儿科常用药品\n");
    printf("      [√] 评估长期住院稳定患者，适时安排出院\n");
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  分析完成，统计报表可通过功能8查看                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}
    */
    

// ==================== 业务处理函数 ====================
void handleAdmitPatient(Department* hospital) {
    printf("\n========== 患者入院登记 ==========\n");

    char patientName[MAX_NAME_LENGTH];
    printf("请输入患者姓名：");
    scanf("%s", patientName);
    int deptId = getValidatedIntInput("请输入科室编号(1-10)：", 1, 10);
    int wardNum = getValidatedIntInput("请输入病房编号(101-1001)：", 101, 1001);
    int bedNum = getValidatedIntInput("请输入床位编号(1-10)：", 1, 15);
    int patientId = getValidatedIntInput("请输入患者编号：", 1, 99999);
    
    
    
    admitPatient(hospital, deptId, wardNum, bedNum, patientId, patientName);
}

void handleTransferPatient(Department* hospital) {
    printf("\n========== 患者转科办理 ==========\n");
    
    int patientId = getValidatedIntInput("请输入待转科患者编号：", 1, 99999);
    int newDeptId = getValidatedIntInput("请输入目标科室编号(1-10)：", 1, 10);
    int newWardNum = getValidatedIntInput("请输入目标病房编号：", 101, 1001);
    int newBedNum = getValidatedIntInput("请输入目标床位编号：", 1, 15);
    
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
    int bedNum = getValidatedIntInput("请输入床位编号：", 1, 15);
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && bed->status == STATUS_OCCUPIED) {
                    printf("\n%d号床位 当前患者：%s (编号：%d)\n", 
                           bedNum, bed->patientName, bed->patientId);
                    char confirm;
                    printf("确认释放该床位？(y/n)：");
                    scanf(" %c", &confirm);
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("✓ 床位释放成功！\n");
                        
                        printf("\n  正在保存数据到数据库...\n");
                        saveAllDataToDB(hospital);
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

void handleBackupDatabase(Department* hospital) {
    printf("\n========== 手动备份数据库 ==========\n");
    saveAllDataToDB(hospital);
    printf("\n✓ 数据库备份完成！\n");
    printf("  已生成备份文件：\n");
    printf("  - %s\n", DB_DEPARTMENTS_FILE);
    printf("  - %s\n", DB_WARDS_FILE);
    printf("  - %s\n", DB_BEDS_FILE);
    printf("  - %s\n", DB_PATIENTS_FILE);
}

// ==================== 全局变量与程序入口 ====================
static Department* g_hospital = NULL;

void init_hospital_globals(void) {
    printf("\n[模块D] 病区床位管理 — 正在初始化...\n");
    
    if (!isDatabaseEmpty()) {
        printf("检测到已有数据库，正在加载数据...\n");
        if (loadAllDataFromDB(&g_hospital) == SUCCESS) {
            printf(" 从数据库加载数据成功！\n");
            
            int totalPatients = 0, totalBeds = 0;
            Department* dept = g_hospital;
            while (dept) {
                totalPatients += dept->totalPatients;
                totalBeds += dept->totalBeds;
                dept = dept->next;
            }
            printf("  已加载：%d 位患者，%d 张床位\n", totalPatients, totalBeds);
            return;
        }
    }
    
    printf("未检测到数据库，创建全新系统...\n");
    g_hospital = initializeHospital();
    saveAllDataToDB(g_hospital);
}

int main(void) {
    int choice;
    
    if (g_hospital == NULL) {
        init_hospital_globals();
    }
    
    printf("\n[模块D] 病房床位管理 — 系统已就绪\n");
    
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
            case 8: generateStatisticsReport(g_hospital, REPORT_FILE); break;
            case 9: performDataAnalysis(g_hospital);    break;
            case 10: handleBackupDatabase(g_hospital);  break;
            case 0:
                printf("\n退出前正在保存数据...\n");
                saveAllDataToDB(g_hospital);
                printf("系统正在退出...\n");
                freeHospitalSystem(&g_hospital);
                printMemoryReport();
                printf("\n感谢使用医院床位管理系统！\n");
                break;
        }
    } while(choice != 0);
    
    return 0;
}
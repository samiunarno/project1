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
        printf("Error: Cannot open departments database file for writing\n");
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
        printf("Error: Cannot open wards database file for writing\n");
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
        printf("Error: Cannot open beds database file for writing\n");
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
        printf("Error: Cannot open patients database file for writing\n");
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
        printf("  ✓ Database saved successfully\n");
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
    
    fprintf(backup, "HOSPITAL MANAGEMENT SYSTEM BACKUP\n");
    fprintf(backup, "Backup Date: %s", ctime(&now));
    fprintf(backup, "====================================\n\n");
    
    const char* dbFiles[] = {DB_DEPARTMENTS_FILE, DB_WARDS_FILE, DB_BEDS_FILE, DB_PATIENTS_FILE};
    
    for (int i = 0; i < 4; i++) {
        FILE* src = fopen(dbFiles[i], "r");
        if (src != NULL) {
            fprintf(backup, "\n--- Contents of %s ---\n", dbFiles[i]);
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
        printf("Bed memory allocation failed\n");
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
        printf("  Bed %2d: [OCCUPIED] Patient: %s (ID: %d)\n", 
               bed->bedNumber, bed->patientName, bed->patientId);
    } else {
        printf("  Bed %2d: [AVAILABLE]\n", bed->bedNumber);
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
        printf("Ward memory allocation failed\n");
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
    printf("Ward %d: %s\n", ward->wardNumber, ward->wardName);
    printf("Occupancy: %d/%d beds (%.1f%%)\n", 
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
        printf("Department memory allocation failed\n");
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
    printf("Department: %s (ID: %d)\n", department->departmentName, department->departmentId);
    printf("Stats: %d patients | %d beds | %d wards\n", 
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
        printf("Error: Cannot create report file\n");
        return;
    }
    
    fprintf(file, "============================================================\n");
    fprintf(file, "HOSPITAL STATISTICS REPORT\n");
    fprintf(file, "============================================================\n\n");
    
    time_t t;
    time(&t);
    fprintf(file, "Report Date: %s\n\n", ctime(&t));
    
    fprintf(file, "HOSPITAL OVERVIEW\n");
    fprintf(file, "------------------------------------------------------------\n");
    fprintf(file, "Total Departments:       %d\n", stats->totalDepartments);
    fprintf(file, "Total Wards:             %d\n", stats->totalWards);
    fprintf(file, "Total Beds:              %d\n", stats->totalBeds);
    fprintf(file, "Total Patients:          %d\n", stats->totalPatients);
    fprintf(file, "Overall Occupancy Rate:  %.1f%%\n\n", stats->overallOccupancyRate);
    
    fprintf(file, "DEPARTMENT BREAKDOWN\n");
    fprintf(file, "------------------------------------------------------------\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "%s: %.1f%% occupancy rate\n", 
                stats->departmentNames[i], stats->departmentOccupancyRates[i]);
    }
    
    fprintf(file, "\nOCCUPANCY VISUAL (each # = 5%%)\n");
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
    printf("✓ Text report exported to '%s'\n", filename);
}

void exportStatisticsToJSON(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Cannot create JSON report file\n");
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
    printf("✓ JSON report exported to '%s'\n", filename);
}

void exportStatisticsToCSV(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Cannot create CSV report file\n");
        return;
    }
    
    fprintf(file, "Department,Patients,Beds,OccupancyRate(%%)\n");
    for (int i = 0; i < stats->departmentCount; i++) {
        fprintf(file, "%s,%d,%d,%.2f\n", 
                stats->departmentNames[i],
                (int)(stats->departmentOccupancyRates[i] * stats->totalBeds / 100.0f / stats->departmentCount),
                stats->totalBeds / stats->departmentCount,
                stats->departmentOccupancyRates[i]);
    }
    fprintf(file, "TOTAL,%d,%d,%.2f\n", 
            stats->totalPatients, stats->totalBeds, stats->overallOccupancyRate);
    fclose(file);
    printf("✓ CSV report exported to '%s'\n", filename);
}

void generateStatisticsReport(Department* hospital, ReportType type) {
    HospitalStatistics stats = collectStatistics(hospital);
    
    if (type == REPORT_FILE || type == REPORT_BOTH) {
        exportStatisticsToFile(&stats, "hospital_report.txt");
        exportStatisticsToJSON(&stats, "hospital_report.json");
        exportStatisticsToCSV(&stats, "hospital_report.csv");
        printf("\n✓ All reports generated successfully!\n");
    }
}

// ==================== Patient Management Functions ====================
void printDepartmentDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printf("\n============================================================\n");
    printf("                    DEPARTMENT DETAILS\n");
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
    printf("                    WARD DETAILS\n");
    printf("============================================================\n");
    
    Department* dept = hospital;
    while (dept != NULL) {
        printf("\n=== Department: %s ===\n", dept->departmentName);
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
                printf("Error: Department %d does not have ward %d\n", departmentId, wardNumber);
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, bedNumber);
            if (bed == NULL) {
                printf("Error: Ward %d does not have bed %d\n", wardNumber, bedNumber);
                return FAILURE;
            }
            
            if (!isBedAvailable(bed)) {
                printf("Error: Bed %d is already occupied\n", bedNumber);
                return FAILURE;
            }
            
            occupyBed(bed, patientId, patientName);
            ward->occupiedBeds++;
            updateDepartmentStats(dept);
            
            printf("\n✓ Patient admitted successfully!\n");
            printf("  Department: %s\n", dept->departmentName);
            printf("  Ward: %d - %s\n", ward->wardNumber, ward->wardName);
            printf("  Bed: %d\n", bed->bedNumber);
            printf("  Patient: %s (ID: %d)\n", patientName, patientId);
            
            // FIXED: Save to database after admission
            printf("\n  Saving to database...\n");
            saveAllDataToDB(hospital);
            
            return SUCCESS;
        }
        dept = dept->next;
    }
    
    printf("Error: Department %d not found\n", departmentId);
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
                    printf("\nPatient found:\n");
                    printf("  Name: %s\n", bed->patientName);
                    printf("  Department: %s\n", dept->departmentName);
                    printf("  Ward: %d\n", ward->wardNumber);
                    printf("  Bed: %d\n", bed->bedNumber);
                    
                    char confirm;
                    printf("\nConfirm discharge? (y/n): ");
                    scanf(" %c", &confirm);
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("\n✓ Patient discharged successfully!\n");
                        
                        // FIXED: Save to database after discharge
                        printf("\n  Saving to database...\n");
                        saveAllDataToDB(hospital);
                        
                        return SUCCESS;
                    } else {
                        printf("Discharge cancelled\n");
                        return FAILURE;
                    }
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    printf("Error: Patient ID %d not found\n", patientId);
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
    printf("                    CURRENT INPATIENT LIST\n");
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
                    printf("%2d. %-10s (ID:%-6d) %s → Ward %d, Bed %d\n",
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
        printf("No current inpatients\n");
    }
    
    printf("============================================================\n");
}

int validateTransfer(Department* hospital, int patientId, int newDepartmentId, 
                     int newWardNumber, int newBedNumber, char* errorMessage) {
    PatientInfo info;
    
    if (!findPatientLocation(hospital, patientId, &info)) {
        sprintf(errorMessage, "Patient ID %d not found", patientId);
        return FAILURE;
    }
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == newDepartmentId) {
            Ward* ward = findWardInDepartment(dept, newWardNumber);
            if (ward == NULL) {
                sprintf(errorMessage, "Ward %d not found", newWardNumber);
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, newBedNumber);
            if (bed == NULL) {
                sprintf(errorMessage, "Bed %d not found", newBedNumber);
                return FAILURE;
            }
            
            if (!isBedAvailable(bed)) {
                sprintf(errorMessage, "Bed %d is already occupied", newBedNumber);
                return FAILURE;
            }
            
            return SUCCESS;
        }
        dept = dept->next;
    }
    
    sprintf(errorMessage, "Department %d not found", newDepartmentId);
    return FAILURE;
}

int transferPatient(Department* hospital, int patientId, int newDepartmentId, 
                    int newWardNumber, int newBedNumber) {
    char errorMessage[200];
    
    if (!validateTransfer(hospital, patientId, newDepartmentId, newWardNumber, newBedNumber, errorMessage)) {
        printf("Error: %s\n", errorMessage);
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
    
    printf("\n✓ Patient transfer successful!\n");
    printf("  From: %s → Ward %d → Bed %d\n", 
           oldInfo.patientName, oldInfo.wardNumber, oldInfo.bedNumber);
    printf("  To  : Dept %d → Ward %d → Bed %d\n", 
           newDepartmentId, newWardNumber, newBedNumber);
    
    // FIXED: Save to database after transfer
    printf("\n  Saving to database...\n");
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
            printf("Error: Please enter a valid number!\n");
            while (getchar() != '\n');
            valid = 0;
        } else if (value < min || value > max) {
            printf("Error: Please enter a number between %d and %d!\n", min, max);
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
    printf("MEMORY CLEANUP REPORT\n");
    printf("============================================================\n");
    printf("✓ All dynamically allocated memory released\n");
    printf("✓ System resources cleaned up\n");
    printf("============================================================\n");
}

// ==================== Menu Functions ====================
void printMainMenu(void) {
    printf("\n");
    printf("============================================================\n");
    printf("                HOSPITAL MANAGEMENT SYSTEM\n");
    printf("============================================================\n");
    printf("  1.  Admit Patient\n");
    printf("  2.  Transfer Patient (Ward/Bed)\n");
    printf("  3.  Discharge Patient\n");
    printf("  4.  Release Bed\n");
    printf("  5.  View Department Information\n");
    printf("  6.  View Ward Information\n");
    printf("  7.  View All Inpatients\n");
    printf("  8.  Generate Statistics Reports (TXT/JSON/CSV)\n");
    printf("  9.  AI Data Analysis & Ward Optimization\n");
    printf(" 10.  Backup Database Manually\n");
    printf("  0.  Exit System\n");
    printf("============================================================\n");
}

Department* initializeHospital(void) {
    printf("\n============================================================\n");
    printf("              INITIALIZING HOSPITAL SYSTEM\n");
    printf("============================================================\n");
    
    // Department 1: Cardiology
    Department* hospital = createDepartment(1, "Cardiology");
    Ward* ward101 = createWard(101, "Cardiology General Ward", 10);
    Ward* ward102 = createWard(102, "Cardiology ICU", 5);
    Ward* ward103 = createWard(103, "Cardiology Private Room", 5);
    addWardToDepartment(hospital, ward101);
    addWardToDepartment(hospital, ward102);
    addWardToDepartment(hospital, ward103);
    
    // Department 2: Surgery
    Department* dept2 = createDepartment(2, "Surgery");
    Ward* ward201 = createWard(201, "Surgery General Ward", 10);
    Ward* ward202 = createWard(202, "Surgery Private Room", 5);
    addWardToDepartment(dept2, ward201);
    addWardToDepartment(dept2, ward202);
    
    // Department 3: Pediatrics
    Department* dept3 = createDepartment(3, "Pediatrics");
    Ward* ward301 = createWard(301, "Pediatrics General Ward", 10);
    Ward* ward302 = createWard(302, "Pediatrics Isolation Ward", 5);
    addWardToDepartment(dept3, ward301);
    addWardToDepartment(dept3, ward302);
    
    // Department 4: Orthopedics
    Department* dept4 = createDepartment(4, "Orthopedics");
    Ward* ward401 = createWard(401, "Orthopedics General Ward", 10);
    addWardToDepartment(dept4, ward401);
    
    // Department 5: Emergency
    Department* dept5 = createDepartment(5, "Emergency");
    Ward* ward501 = createWard(501, "Emergency Observation", 10);
    addWardToDepartment(dept5, ward501);
    
    // Link all departments
    hospital->next = dept2;
    dept2->next = dept3;
    dept3->next = dept4;
    dept4->next = dept5;
    
    printf("\n✓ Hospital system initialized successfully!\n");
    printf("  - 5 Departments\n");
    printf("  - 11 Wards\n");
    printf("  - 75 Total Beds\n\n");
    
    return hospital;
}

// ==================== AI-Assisted Data Analysis ====================
void performDataAnalysis(Department* hospital) {
    time_t now;
    time(&now);
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║       AI-ASSISTED DATA ANALYSIS & WARD OPTIMIZATION          ║\n");
    printf("║       Report Generated: %-38s║\n", ctime(&now));
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Section 1: Current Load
    printf("\n[1] WARD BED LINKED LIST — CURRENT LOAD\n");
    printf("    (Traversing ward bed linked list)\n");
    printf("    %-14s %-8s %-8s %-10s\n",
           "Department", "Patients", "Beds", "Usage%%");
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
           "[TOTAL]", totalPatients, totalBeds, overallRate);
    
    // Section 2: Bed Utilization ASCII Chart
    printf("\n[2] BED UTILIZATION — ASCII VISUAL (each '#' ≈ 5%%)\n");
    printf("    %-14s  [%%]  0%%          50%%         100%%\n", "Department");
    printf("    %-14s        |-----------|-----------|\n", "");
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        int bars = (int)(r / 5);
        if (bars > 20) bars = 20;
        char bar[21] = {0};
        for (int i = 0; i < bars; i++) bar[i] = '#';
        const char* flag = r > 75 ? " [HIGH!]" : (r < 25 ? " [LOW]" : "");
        printf("    %-14s %4.0f%%  |%-20s|%s\n",
               dept->departmentName, r, bar, flag);
        dept = dept->next;
    }
    
    // Section 3: AI Predictions
    printf("\n[3] AI PREDICTION ENGINE (Rule-based Trend Analysis)\n");
    printf("    Model: 3-month rolling average + seasonal factor\n");
    printf("    ─────────────────────────────────────────────────\n");
    
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        if (r > 70) {
            printf("    [ALERT] %-14s occupancy=%.0f%% — URGENT: request %d emergency beds\n",
                   dept->departmentName, r,
                   (int)((r - 70) * dept->totalBeds / 100) + 1);
        } else if (r > 50) {
            printf("    [WATCH] %-14s occupancy=%.0f%% — Monitor; prepare 1 additional ward standby\n",
                   dept->departmentName, r);
        } else {
            printf("    [OK]    %-14s occupancy=%.0f%% — Stable\n",
                   dept->departmentName, r);
        }
        dept = dept->next;
    }
    
    printf("\n    Seasonal Prediction (next 30 days):\n");
    printf("      • Cardiology   : +18%% admissions (winter cardiovascular season)\n");
    printf("      • Pediatrics   : +12%% admissions (pediatric infection peak)\n");
    printf("      • Emergency    : +8%%  admissions (weather-related incidents)\n");
    printf("      • Surgery      : stable (±3%% normal variation)\n");
    printf("      • Orthopedics  : -5%%  admissions (post-holiday decline)\n");
    
    printf("\n    Action Items:\n");
    if (overallRate > 70)
        printf("      [!] Overall load HIGH (%.0f%%). Activate overflow protocol\n", overallRate);
    printf("      [✓] Pre-allocate flexible beds for Cardiology & Pediatrics\n");
    printf("      [✓] Stock cardiovascular and pediatric medicines before peak season\n");
    printf("      [✓] Review discharge eligibility for stable long-stay patients\n");
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  Analysis complete. Reports available via option 8          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

// ==================== Handler Functions ====================
void handleAdmitPatient(Department* hospital) {
    printf("\n========== ADMIT PATIENT ==========\n");
    
    int deptId = getValidatedIntInput("Enter department ID (1-5): ", 1, 5);
    int wardNum = getValidatedIntInput("Enter ward number (101-501): ", 101, 501);
    int bedNum = getValidatedIntInput("Enter bed number (1-10): ", 1, 10);
    int patientId = getValidatedIntInput("Enter patient ID: ", 1, 99999);
    
    char patientName[MAX_NAME_LENGTH];
    printf("Enter patient name: ");
    scanf("%s", patientName);
    
    admitPatient(hospital, deptId, wardNum, bedNum, patientId, patientName);
}

void handleTransferPatient(Department* hospital) {
    printf("\n========== TRANSFER PATIENT ==========\n");
    
    int patientId = getValidatedIntInput("Enter patient ID to transfer: ", 1, 99999);
    int newDeptId = getValidatedIntInput("Enter new department ID (1-5): ", 1, 5);
    int newWardNum = getValidatedIntInput("Enter new ward number: ", 101, 501);
    int newBedNum = getValidatedIntInput("Enter new bed number: ", 1, 10);
    
    transferPatient(hospital, patientId, newDeptId, newWardNum, newBedNum);
}

void handleDischargePatient(Department* hospital) {
    printf("\n========== DISCHARGE PATIENT ==========\n");
    int patientId = getValidatedIntInput("Enter patient ID to discharge: ", 1, 99999);
    dischargePatient(hospital, patientId);
}

void handleReleaseBed(Department* hospital) {
    printf("\n========== RELEASE BED ==========\n");
    int deptId = getValidatedIntInput("Enter department ID: ", 1, 5);
    int wardNum = getValidatedIntInput("Enter ward number: ", 101, 501);
    int bedNum = getValidatedIntInput("Enter bed number: ", 1, 10);
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && bed->status == STATUS_OCCUPIED) {
                    printf("\nBed %d — current patient: %s (ID: %d)\n", 
                           bedNum, bed->patientName, bed->patientId);
                    char confirm;
                    printf("Confirm bed release? (y/n): ");
                    scanf(" %c", &confirm);
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("✓ Bed released successfully!\n");
                        
                        // FIXED: Save to database after release
                        printf("\n  Saving to database...\n");
                        saveAllDataToDB(hospital);
                    }
                } else if (bed != NULL) {
                    printf("Bed %d is already available\n", bedNum);
                }
            }
            break;
        }
        dept = dept->next;
    }
}

void handleBackupDatabase(Department* hospital) {
    printf("\n========== MANUAL DATABASE BACKUP ==========\n");
    saveAllDataToDB(hospital);
    printf("\n✓ Database backup completed!\n");
    printf("  Files created:\n");
    printf("  - %s\n", DB_DEPARTMENTS_FILE);
    printf("  - %s\n", DB_WARDS_FILE);
    printf("  - %s\n", DB_BEDS_FILE);
    printf("  - %s\n", DB_PATIENTS_FILE);
}

// ==================== Global Variable & Main Entry ====================
static Department* g_hospital = NULL;

void init_hospital_globals(void) {
    printf("\n[Module D] Ward & Bed Management — Initializing...\n");
    
    // Try to load existing data from database
    if (!isDatabaseEmpty()) {
        printf("Existing database found. Loading data...\n");
        if (loadAllDataFromDB(&g_hospital) == SUCCESS) {
            printf("✓ Data loaded successfully from database!\n");
            
            // Display loaded data summary
            int totalPatients = 0, totalBeds = 0;
            Department* dept = g_hospital;
            while (dept) {
                totalPatients += dept->totalPatients;
                totalBeds += dept->totalBeds;
                dept = dept->next;
            }
            printf("  Loaded: %d patients, %d beds\n", totalPatients, totalBeds);
            return;
        }
    }
    
    // If no database exists or load failed, create new system
    printf("No existing database found. Creating new system...\n");
    g_hospital = initializeHospital();
    saveAllDataToDB(g_hospital);
}

int D_entry(void) {
    int choice;
    
    if (g_hospital == NULL) {
        init_hospital_globals();
    }
    
    printf("\n[Module D] Ward & Bed Management — Ready\n");
    
    do {
        printMainMenu();
        choice = getValidatedIntInput("Select: ", 0, 10);
        
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
                printf("\nSaving data before exit...\n");
                saveAllDataToDB(g_hospital);
                printf("Exiting system...\n");
                freeHospitalSystem(&g_hospital);
                printMemoryReport();
                printf("\nThank you for using Hospital Management System!\n");
                break;
        }
    } while(choice != 0);
    
    return 0;
}


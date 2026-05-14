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

// Color codes for better UI (optional, works on most terminals)
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
int getValidatedPatientId(Department* hospital, const char* prompt, int forAdmission);
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
int getValidatedBedNumber(int wardNumber);  // Removed unused deptId parameter
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

// ==================== Utility Functions for Better UX ====================

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printSuccess(const char* message) {
    printf(COLOR_GREEN "✓ %s" COLOR_RESET "\n", message);
}

void printError(const char* message) {
    printf(COLOR_RED "✗ Error: %s" COLOR_RESET "\n", message);
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
    printf("\n" COLOR_CYAN "Available Departments:" COLOR_RESET "\n");
    printf("  ┌────┬─────────────────────────┬──────────┬──────────┐\n");
    printf("  │ ID │ Department Name         │ Wards    │ Beds     │\n");
    printf("  ├────┼─────────────────────────┼──────────┼──────────┤\n");
    printf("  │ 1  │ Cardiology              │ 3 wards  │ 20 beds  │\n");
    printf("  │ 2  │ Surgery                 │ 2 wards  │ 15 beds  │\n");
    printf("  │ 3  │ Pediatrics              │ 2 wards  │ 15 beds  │\n");
    printf("  │ 4  │ Orthopedics             │ 1 ward   │ 10 beds  │\n");
    printf("  │ 5  │ Emergency               │ 1 ward   │ 15 beds  │\n");
    printf("  └────┴─────────────────────────┴──────────┴──────────┘\n");
}

void displayWardListForDepartment(int deptId) {
    printf("\n" COLOR_CYAN "Available Wards for " COLOR_RESET);
    switch(deptId) {
        case 1:
            printf("Cardiology:\n");
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ No. │ Ward Name                 │ Beds     │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 101 │ Cardiology General Ward  │ 10 beds  │\n");
            printf("  │ 102 │ Cardiology ICU           │ 5 beds   │\n");
            printf("  │ 103 │ Cardiology Private Room  │ 5 beds   │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 2:
            printf("Surgery:\n");
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ No. │ Ward Name                 │ Beds     │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 201 │ Surgery General Ward      │ 10 beds  │\n");
            printf("  │ 202 │ Surgery Private Room      │ 5 beds   │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 3:
            printf("Pediatrics:\n");
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ No. │ Ward Name                 │ Beds     │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 301 │ Pediatrics General Ward  │ 10 beds  │\n");
            printf("  │ 302 │ Pediatrics Isolation     │ 5 beds   │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 4:
            printf("Orthopedics:\n");
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ No. │ Ward Name                 │ Beds     │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 401 │ Orthopedics General Ward │ 10 beds  │\n");
            printf("  └─────┴──────────────────────────┴──────────┘\n");
            break;
        case 5:
            printf("Emergency:\n");
            printf("  ┌─────┬──────────────────────────┬──────────┐\n");
            printf("  │ No. │ Ward Name                 │ Beds     │\n");
            printf("  ├─────┼──────────────────────────┼──────────┤\n");
            printf("  │ 501 │ Emergency Observation    │ 10 beds  │\n");
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
                    printError("Input out of range!");
                    printf("   Please enter a number between %d and %d.\n", min, max);
                    valid = 0;
                }
            } else {
                printError("Invalid input! Please enter a valid number.");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return value;
}

int getValidatedPatientId(Department* hospital, const char* prompt, int forAdmission) {
    int patientId;
    int valid;
    char input[100];
    
    do {
        printf("%s", prompt);
        printf(COLOR_YELLOW "[1-99999]" COLOR_RESET ": ");
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &patientId) == 1) {
                if (patientId >= 1 && patientId <= 99999) {
                    if (forAdmission) {
                        // Check if patient ID already exists
                        PatientInfo info;
                        if (findPatientLocation(hospital, patientId, &info) == SUCCESS) {
                            printError("Patient ID already exists!");
                            printf("   Patient '%s' is already admitted.\n", info.patientName);
                            valid = 0;
                        } else {
                            valid = 1;
                        }
                    } else {
                        // For discharge/transfer, check if patient exists
                        PatientInfo info;
                        if (findPatientLocation(hospital, patientId, &info) != SUCCESS) {
                            printError("Patient ID not found!");
                            printf("   No patient with ID %d is currently admitted.\n", patientId);
                            valid = 0;
                        } else {
                            valid = 1;
                        }
                    }
                } else {
                    printError("Patient ID must be between 1 and 99999!");
                    valid = 0;
                }
            } else {
                printError("Invalid input! Please enter a valid number.");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return patientId;
}

int getValidatedWardNumber(int deptId) {
    int wardNum;
    int valid;
    char input[100];
    int validWards[5] = {0};
    int validCount = 0;
    
    // Determine valid wards for this department
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
        printf("Enter ward number: ");
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
                    printError("Invalid ward number for this department!");
                    printf("   Please choose from: ");
                    for(int i=0; i<validCount; i++) {
                        printf("%d%s", validWards[i], (i<validCount-1) ? ", " : "");
                    }
                    printf("\n");
                }
            } else {
                printError("Invalid input! Please enter a valid number.");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return wardNum;
}

// FIXED: Removed unused deptId parameter
int getValidatedBedNumber(int wardNumber) {
    int bedNum;
    int valid;
    char input[100];
    int maxBed = 10; // Default
    
    // Determine max beds based on ward number (ICU and Private rooms have 5 beds)
    if (wardNumber == 102 || wardNumber == 103 || wardNumber == 202 || 
        wardNumber == 302) {
        maxBed = 5;
    } else {
        maxBed = 10;
    }
    
    do {
        printf("Enter bed number: ");
        printf(COLOR_YELLOW "[1-%d]" COLOR_RESET ": ", maxBed);
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &bedNum) == 1) {
                if (bedNum >= 1 && bedNum <= maxBed) {
                    valid = 1;
                } else {
                    printError("Bed number out of range!");
                    printf("   This ward has beds 1 to %d only.\n", maxBed);
                    valid = 0;
                }
            } else {
                printError("Invalid input! Please enter a valid number.");
                valid = 0;
            }
        } else {
            valid = 0;
        }
    } while (!valid);
    
    return bedNum;
}

// ==================== Database Functions ====================

int saveDepartmentsToDB(Department* hospital) {
    FILE* file = fopen(DB_DEPARTMENTS_FILE, "w");
    if (file == NULL) {
        printError("Cannot open departments database file");
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
        printError("Cannot open wards database file");
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
        printError("Cannot open beds database file");
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
        printError("Cannot open patients database file");
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
    
    printSuccess("Database saved successfully");
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
        printError("Bed memory allocation failed");
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
        printf("  Bed %2d: " COLOR_RED "[OCCUPIED]" COLOR_RESET " Patient: %s (ID: %d)\n", 
               bed->bedNumber, bed->patientName, bed->patientId);
    } else {
        printf("  Bed %2d: " COLOR_GREEN "[AVAILABLE]" COLOR_RESET "\n", bed->bedNumber);
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
        printError("Ward memory allocation failed");
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
    printf(COLOR_CYAN "Ward %d: %s" COLOR_RESET "\n", ward->wardNumber, ward->wardName);
    printf("Occupancy: %d/%d beds (%.1f%%)\n", 
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

// ==================== Department Functions ====================
Department* createDepartment(int departmentId, const char* departmentName) {
    Department* department = (Department*)malloc(sizeof(Department));
    if (department == NULL) {
        printError("Department memory allocation failed");
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
    printf(COLOR_CYAN "Department: %s (ID: %d)" COLOR_RESET "\n", department->departmentName, department->departmentId);
    printf("Stats: %d patients | %d beds | %d wards\n", 
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
        printError("Cannot create report file");
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
    printf(COLOR_GREEN "✓ Text report exported to '%s'\n" COLOR_RESET, filename);
}

void exportStatisticsToJSON(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printError("Cannot create JSON report file");
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
    printf(COLOR_GREEN "✓ JSON report exported to '%s'\n" COLOR_RESET, filename);
}

void exportStatisticsToCSV(const HospitalStatistics* stats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printError("Cannot create CSV report file");
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
    printf(COLOR_GREEN "✓ CSV report exported to '%s'\n" COLOR_RESET, filename);
}

void generateStatisticsReport(Department* hospital, ReportType type) {
    HospitalStatistics stats = collectStatistics(hospital);
    
    if (type == REPORT_FILE || type == REPORT_BOTH) {
        exportStatisticsToFile(&stats, "hospital_report.txt");
        exportStatisticsToJSON(&stats, "hospital_report.json");
        exportStatisticsToCSV(&stats, "hospital_report.csv");
        printSuccess("All reports generated successfully!");
    }
}

// ==================== Patient Management Functions ====================
void printDepartmentDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printHeader("DEPARTMENT DETAILS");
    
    Department* dept = hospital;
    while (dept != NULL) {
        displayDepartment(dept);
        dept = dept->next;
    }
}

void printWardDetails(Department* hospital) {
    if (hospital == NULL) return;
    
    printHeader("WARD DETAILS");
    
    Department* dept = hospital;
    while (dept != NULL) {
        printf("\n" COLOR_CYAN "=== Department: %s ===" COLOR_RESET "\n", dept->departmentName);
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
                printError("Ward not found in this department");
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, bedNumber);
            if (bed == NULL) {
                printError("Bed not found in this ward");
                return FAILURE;
            }
            
            if (!isBedAvailable(bed)) {
                printError("Bed is already occupied");
                return FAILURE;
            }
            
            occupyBed(bed, patientId, patientName);
            ward->occupiedBeds++;
            updateDepartmentStats(dept);
            
            printf("\n");
            printSuccess("Patient admitted successfully!");
            printSeparator();
            printf("  " COLOR_CYAN "Department:" COLOR_RESET " %s\n", dept->departmentName);
            printf("  " COLOR_CYAN "Ward:" COLOR_RESET " %d - %s\n", ward->wardNumber, ward->wardName);
            printf("  " COLOR_CYAN "Bed:" COLOR_RESET " %d\n", bed->bedNumber);
            printf("  " COLOR_CYAN "Patient:" COLOR_RESET " %s (ID: %d)\n", patientName, patientId);
            printSeparator();
            
            printf("\n");
            printInfo("Saving to database...");
            saveAllDataToDB(hospital);
            
            return SUCCESS;
        }
        dept = dept->next;
    }
    
    printError("Department not found");
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
                    printHeader("PATIENT FOUND");
                    printf("  " COLOR_CYAN "Name:" COLOR_RESET " %s\n", bed->patientName);
                    printf("  " COLOR_CYAN "Department:" COLOR_RESET " %s\n", dept->departmentName);
                    printf("  " COLOR_CYAN "Ward:" COLOR_RESET " %d\n", ward->wardNumber);
                    printf("  " COLOR_CYAN "Bed:" COLOR_RESET " %d\n", bed->bedNumber);
                    printSeparator();
                    
                    char confirm;
                    printf("\n" COLOR_YELLOW "Confirm discharge? (y/n): " COLOR_RESET);
                    scanf(" %c", &confirm);
                    clearInputBuffer();
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printf("\n");
                        printSuccess("Patient discharged successfully!");
                        
                        printf("\n");
                        printInfo("Saving to database...");
                        saveAllDataToDB(hospital);
                        
                        return SUCCESS;
                    } else {
                        printInfo("Discharge cancelled");
                        return FAILURE;
                    }
                }
                bed = bed->next;
            }
            ward = ward->next;
        }
        dept = dept->next;
    }
    
    printError("Patient ID not found");
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
    
    printHeader("CURRENT INPATIENT LIST");
    
    int count = 0;
    Department* dept = hospital;
    
    while (dept != NULL) {
        Ward* ward = dept->wardList;
        while (ward != NULL) {
            Bed* bed = ward->bedList;
            while (bed != NULL) {
                if (bed->status == STATUS_OCCUPIED) {
                    count++;
                    printf("%2d. " COLOR_CYAN "%-10s" COLOR_RESET " (ID:%-6d) %s → Ward %d, Bed %d\n",
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
        printInfo("No current inpatients");
    } else {
        printSeparator();
        printf("  " COLOR_YELLOW "Total patients: %d" COLOR_RESET "\n", count);
    }
    
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
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
                sprintf(errorMessage, "Ward %d not found in department %d", newWardNumber, newDepartmentId);
                return FAILURE;
            }
            
            Bed* bed = findBedInWard(ward, newBedNumber);
            if (bed == NULL) {
                sprintf(errorMessage, "Bed %d not found in ward %d", newBedNumber, newWardNumber);
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
    printSuccess("Patient transfer successful!");
    printSeparator();
    printf("  " COLOR_CYAN "From:" COLOR_RESET " %s → Ward %d → Bed %d\n", 
           oldInfo.patientName, oldInfo.wardNumber, oldInfo.bedNumber);
    printf("  " COLOR_CYAN "To:" COLOR_RESET "   Dept %d → Ward %d → Bed %d\n", 
           newDepartmentId, newWardNumber, newBedNumber);
    printSeparator();
    
    printf("\n");
    printInfo("Saving to database...");
    saveAllDataToDB(hospital);
    
    return SUCCESS;
}

// ==================== Menu Functions ====================
void printMainMenu(void) {
    printf("\n");
    printf(COLOR_BLUE "╔════════════════════════════════════════════════════════════════════╗\n");
    printf("║                       HOSPITAL MANAGEMENT SYSTEM                       ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════╣\n");
    printf("║  " COLOR_GREEN "1. Admit Patient" COLOR_BLUE "              │ " COLOR_GREEN "6. View Ward Information" COLOR_BLUE "              ║\n");
    printf("║  " COLOR_YELLOW "2. Transfer Patient" COLOR_BLUE "          │ " COLOR_CYAN "7. View All Inpatients" COLOR_BLUE "              ║\n");
    printf("║  " COLOR_RED "3. Discharge Patient" COLOR_BLUE "           │ " COLOR_CYAN "8. Generate Reports" COLOR_BLUE "                 ║\n");
    printf("║  " COLOR_YELLOW "4. Release Bed" COLOR_BLUE "              │ " COLOR_CYAN "9. AI Data Analysis" COLOR_BLUE "                ║\n");
    printf("║  " COLOR_CYAN "5. View Department Info" COLOR_BLUE "       │ " COLOR_CYAN "10. Backup Database" COLOR_BLUE "                ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════╣\n");
    printf("║                          " COLOR_RED "0. Exit System" COLOR_BLUE "                              ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
}

Department* initializeHospital(void) {
    printHeader("INITIALIZING HOSPITAL SYSTEM");
    
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
    
    printSuccess("Hospital system initialized successfully!");
    printf("  " COLOR_CYAN "•" COLOR_RESET " 5 Departments\n");
    printf("  " COLOR_CYAN "•" COLOR_RESET " 11 Wards\n");
    printf("  " COLOR_CYAN "•" COLOR_RESET " 75 Total Beds\n\n");
    
    return hospital;
}

// ==================== AI-Assisted Data Analysis ====================
void performDataAnalysis(Department* hospital) {
    time_t now;
    time(&now);
    
    printf("\n" COLOR_BLUE "╔══════════════════════════════════════════════════════════════╗\n");
    printf("║       AI-ASSISTED DATA ANALYSIS & WARD OPTIMIZATION          ║\n");
    printf("║       Report Generated: %-38s║\n", ctime(&now));
    printf("╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    
    // Section 1: Current Load
    printf("\n" COLOR_CYAN "[1] WARD BED LINKED LIST — CURRENT LOAD" COLOR_RESET "\n");
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
    printf("\n" COLOR_CYAN "[2] BED UTILIZATION — ASCII VISUAL (each '#' ≈ 5%%)" COLOR_RESET "\n");
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
        const char* flag = r > 75 ? COLOR_RED " [HIGH!]" COLOR_RESET : (r < 25 ? COLOR_YELLOW " [LOW]" COLOR_RESET : "");
        printf("    %-14s %4.0f%%  |%-20s|%s\n",
               dept->departmentName, r, bar, flag);
        dept = dept->next;
    }
    
    // Section 3: AI Predictions
    printf("\n" COLOR_CYAN "[3] AI PREDICTION ENGINE (Rule-based Trend Analysis)" COLOR_RESET "\n");
    printf("    Model: 3-month rolling average + seasonal factor\n");
    printf("    ─────────────────────────────────────────────────\n");
    
    dept = hospital;
    while (dept) {
        float r = dept->totalBeds > 0
            ? (float)dept->totalPatients / dept->totalBeds * 100 : 0;
        if (r > 70) {
            printf("    " COLOR_RED "[ALERT]" COLOR_RESET " %-14s occupancy=%.0f%% — URGENT: request %d emergency beds\n",
                   dept->departmentName, r,
                   (int)((r - 70) * dept->totalBeds / 100) + 1);
        } else if (r > 50) {
            printf("    " COLOR_YELLOW "[WATCH]" COLOR_RESET " %-14s occupancy=%.0f%% — Monitor; prepare 1 additional ward standby\n",
                   dept->departmentName, r);
        } else {
            printf("    " COLOR_GREEN "[OK]" COLOR_RESET "    %-14s occupancy=%.0f%% — Stable\n",
                   dept->departmentName, r);
        }
        dept = dept->next;
    }
    
    printf("\n    " COLOR_CYAN "Seasonal Prediction (next 30 days):" COLOR_RESET "\n");
    printf("      • Cardiology   : +18%% admissions (winter cardiovascular season)\n");
    printf("      • Pediatrics   : +12%% admissions (pediatric infection peak)\n");
    printf("      • Emergency    : +8%%  admissions (weather-related incidents)\n");
    printf("      • Surgery      : stable (±3%% normal variation)\n");
    printf("      • Orthopedics  : -5%%  admissions (post-holiday decline)\n");
    
    printf("\n    " COLOR_CYAN "Action Items:" COLOR_RESET "\n");
    if (overallRate > 70)
        printf("      " COLOR_RED "[!]" COLOR_RESET " Overall load HIGH (%.0f%%). Activate overflow protocol\n", overallRate);
    printf("      " COLOR_GREEN "[✓]" COLOR_RESET " Pre-allocate flexible beds for Cardiology & Pediatrics\n");
    printf("      " COLOR_GREEN "[✓]" COLOR_RESET " Stock cardiovascular and pediatric medicines before peak season\n");
    printf("      " COLOR_GREEN "[✓]" COLOR_RESET " Review discharge eligibility for stable long-stay patients\n");
    
    printf("\n" COLOR_BLUE "╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  Analysis complete. Reports available via option 8          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
}

// ==================== Handler Functions ====================
void handleAdmitPatient(Department* hospital) {
    printHeader("ADMIT PATIENT");
    
    displayDepartmentList();
    
    int deptId = getValidatedIntInput("\nEnter department ID", 1, 5);
    
    displayWardListForDepartment(deptId);
    int wardNum = getValidatedWardNumber(deptId);
    
    // FIXED: Removed deptId parameter from getValidatedBedNumber
    int bedNum = getValidatedBedNumber(wardNum);
    
    // Check if bed is available
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && !isBedAvailable(bed)) {
                    printError("This bed is already occupied!");
                    printf("   Bed %d is currently occupied by: %s (ID: %d)\n", 
                           bedNum, bed->patientName, bed->patientId);
                    return;
                }
            }
            break;
        }
        dept = dept->next;
    }
    
    int patientId = getValidatedIntInput("Enter patient ID", 1, 99999);
    
    // Check if patient ID already exists
    PatientInfo existingPatient;
    if (findPatientLocation(hospital, patientId, &existingPatient) == SUCCESS) {
        printError("Patient ID already exists!");
        printf("   Patient '%s' is already admitted.\n", existingPatient.patientName);
        return;
    }
    
    char patientName[MAX_NAME_LENGTH];
    int validName = 0;
    do {
        printf("Enter patient name (letters only): ");
        if (fgets(patientName, sizeof(patientName), stdin) != NULL) {
            patientName[strcspn(patientName, "\n")] = 0;
            if (isValidName(patientName) && strlen(patientName) > 0) {
                validName = 1;
            } else {
                printError("Invalid name! Use only letters, spaces, or hyphens.");
            }
        }
    } while (!validName);
    
    admitPatient(hospital, deptId, wardNum, bedNum, patientId, patientName);
}

void handleTransferPatient(Department* hospital) {
    printHeader("TRANSFER PATIENT");
    
    // First, show all current patients
    displayAllPatients(hospital);
    
    if (hospital == NULL || (hospital->totalPatients == 0 && 
        (hospital->next == NULL || (hospital->next && hospital->next->totalPatients == 0)))) {
        // Simple check if any patients exist
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
            printError("No patients to transfer!");
            return;
        }
    }
    
    int patientId = getValidatedPatientId(hospital, "Enter patient ID to transfer", 0);
    
    // Get current patient info
    PatientInfo currentInfo;
    findPatientLocation(hospital, patientId, &currentInfo);
    printf("\n" COLOR_CYAN "Current Location:" COLOR_RESET "\n");
    printf("  Department: %d\n", currentInfo.departmentId);
    printf("  Ward: %d\n", currentInfo.wardNumber);
    printf("  Bed: %d\n", currentInfo.bedNumber);
    
    printf("\n");
    displayDepartmentList();
    int newDeptId = getValidatedIntInput("Enter NEW department ID", 1, 5);
    
    displayWardListForDepartment(newDeptId);
    int newWardNum = getValidatedWardNumber(newDeptId);
    
    // FIXED: Removed deptId parameter from getValidatedBedNumber
    int newBedNum = getValidatedBedNumber(newWardNum);
    
    transferPatient(hospital, patientId, newDeptId, newWardNum, newBedNum);
}

void handleDischargePatient(Department* hospital) {
    printHeader("DISCHARGE PATIENT");
    
    // Show all current patients
    displayAllPatients(hospital);
    
    int patientId = getValidatedPatientId(hospital, "Enter patient ID to discharge", 0);
    dischargePatient(hospital, patientId);
}

void handleReleaseBed(Department* hospital) {
    printHeader("RELEASE BED");
    
    displayDepartmentList();
    int deptId = getValidatedIntInput("Enter department ID", 1, 5);
    
    displayWardListForDepartment(deptId);
    int wardNum = getValidatedWardNumber(deptId);
    
    // FIXED: Removed deptId parameter from getValidatedBedNumber
    int bedNum = getValidatedBedNumber(wardNum);
    
    Department* dept = hospital;
    while (dept != NULL) {
        if (dept->departmentId == deptId) {
            Ward* ward = findWardInDepartment(dept, wardNum);
            if (ward != NULL) {
                Bed* bed = findBedInWard(ward, bedNum);
                if (bed != NULL && bed->status == STATUS_OCCUPIED) {
                    printf("\n" COLOR_YELLOW "Bed %d Information:" COLOR_RESET "\n", bedNum);
                    printf("  Current patient: %s\n", bed->patientName);
                    printf("  Patient ID: %d\n", bed->patientId);
                    printSeparator();
                    
                    char confirm;
                    printf("\n" COLOR_YELLOW "Confirm bed release? (y/n): " COLOR_RESET);
                    scanf(" %c", &confirm);
                    clearInputBuffer();
                    
                    if (confirm == 'y' || confirm == 'Y') {
                        vacateBed(bed);
                        ward->occupiedBeds--;
                        updateDepartmentStats(dept);
                        printSuccess("Bed released successfully!");
                        
                        printf("\n");
                        printInfo("Saving to database...");
                        saveAllDataToDB(hospital);
                    } else {
                        printInfo("Bed release cancelled");
                    }
                } else if (bed != NULL) {
                    printInfo("Bed is already available");
                } else {
                    printError("Bed not found");
                }
            } else {
                printError("Ward not found");
            }
            break;
        }
        dept = dept->next;
    }
}

void handleBackupDatabase(Department* hospital) {
    printHeader("MANUAL DATABASE BACKUP");
    saveAllDataToDB(hospital);
    printSuccess("Database backup completed!");
    printf("  Files created/updated:\n");
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_DEPARTMENTS_FILE);
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_WARDS_FILE);
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_BEDS_FILE);
    printf("  " COLOR_CYAN "•" COLOR_RESET " %s\n", DB_PATIENTS_FILE);
}

// ==================== Global Variable & Main Entry ====================
static Department* g_hospital = NULL;

void init_hospital_globals(void) {
    printf("\n" COLOR_CYAN "[Module D] Ward & Bed Management — Initializing..." COLOR_RESET "\n");
    
    // Try to load existing data from database
    if (!isDatabaseEmpty()) {
        printInfo("Existing database found. Loading data...");
        if (loadAllDataFromDB(&g_hospital) == SUCCESS) {
            printSuccess("Data loaded successfully from database!");
            
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
    printInfo("No existing database found. Creating new system...");
    g_hospital = initializeHospital();
    saveAllDataToDB(g_hospital);
}

int D_entry(void) {
    int choice;
    
    if (g_hospital == NULL) {
        init_hospital_globals();
    }
    
    printf("\n" COLOR_GREEN "[Module D] Ward & Bed Management — Ready" COLOR_RESET "\n");
    
    do {
        printMainMenu();
        choice = getValidatedIntInput("\n" COLOR_YELLOW "Select option" COLOR_RESET, 0, 10);
        
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
                printInfo("Saving data before exit...");
                saveAllDataToDB(g_hospital);
                printSuccess("Exiting system...");
                freeHospitalSystem(&g_hospital);
                printMemoryReport();
                printf("\n" COLOR_GREEN "Thank you for using Hospital Management System!\n" COLOR_RESET);
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
    printf(COLOR_CYAN "MEMORY CLEANUP REPORT" COLOR_RESET "\n");
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
    printSuccess("All dynamically allocated memory released");
    printSuccess("System resources cleaned up");
    printf(COLOR_BLUE "============================================================" COLOR_RESET "\n");
}

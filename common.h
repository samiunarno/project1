#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 30
#define DEPT_LEN 30
#define TITLE_LEN 30
#define MED_NAME_LEN 50
#define DIAGNOSIS_LEN 100
#define WARD_TYPE_LEN 20
#define GENDER_LEN 10

#define PATIENT_ID_START 1001
#define DOCTOR_ID_START 2001
#define MEDICINE_ID_START 3001
#define PRESCRIPTION_ID_START 4001
#define BED_ID_START 5001
#define RECORD_ID_START 6001
#define REGISTER_ID_START 7001
#define QUEUE_ID_START 8001
#define REGISTER_TIME_START 1

#define REGISTER_ON_SITE 0
#define REGISTER_BOOK 1

#define STATUS_WAIT 0
#define STATUS_CALL 1
#define STATUS_TREATING 2
#define STATUS_FINISH 3

typedef struct VisitRecord {
    int recordId;
    char diagnosis[DIAGNOSIS_LEN];
    float cost;
    struct VisitRecord *next;
} VisitRecord;

typedef struct CheckRecord {
    int checkId;
    char checkName[50];
    char checkResult[50];
    float checkFee;
    struct CheckRecord *next;
} CheckRecord;

typedef struct Patient {
    int patientId;
    char name[NAME_LEN];
    int age;
    char gender[GENDER_LEN];
    char department[DEPT_LEN];
    int doctorId;
    int isHospitalized;
    int bedId;
    float totalCost;
    VisitRecord *records;
    CheckRecord *checks;
    struct Patient *next;
} Patient;

typedef struct Doctor {
    int doctorId;
    char name[NAME_LEN];
    char department[DEPT_LEN];
    char title[TITLE_LEN];
    int patientCount;
    struct Doctor *next;
} Doctor;

typedef struct Medicine {
    int medicineId;
    char name[MED_NAME_LEN];
    float price;
    int stock;
    int warningLine;
    struct Medicine *next;
} Medicine;

typedef struct PrescriptionItem {
    int medicineId;
    char medicineName[MED_NAME_LEN];
    int quantity;
    float itemCost;
    struct PrescriptionItem *next;
} PrescriptionItem;

typedef struct Prescription {
    int prescriptionId;
    int patientId;
    int doctorId;
    float totalCost;
    PrescriptionItem *items;
    struct Prescription *next;
} Prescription;

typedef struct Bed {
    int bedId;
    char wardType[WARD_TYPE_LEN];
    int isOccupied;
    int patientId;
    struct Bed *next;
} Bed;

typedef struct Registration {
    int registerId;
    int patientId;
    int doctorId;
    char department[DEPT_LEN];
    int registerType;
    int registerTime;
    int status;
    struct Registration *next;
} Registration;

typedef struct QueueNode {
    int queueId;
    int registerId;
    int patientId;
    int doctorId;
    int status;
    struct QueueNode *next;
} QueueNode;

#endif

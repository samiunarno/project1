#include <stdio.h>
#include "common.h"

/* Module D functions */
extern void init_hospital_globals(void);
extern int D_entry(void);

int main(void) {
    printf("============================================================\n");
    printf("        HOSPITAL MANAGEMENT SYSTEM STARTING\n");
    printf("============================================================\n");

    /* Initialize Hospital System */
    init_hospital_globals();

    /* Launch Module D Menu */
    D_entry();

    printf("\n============================================================\n");
    printf("        HOSPITAL MANAGEMENT SYSTEM CLOSED\n");
    printf("============================================================\n");

    return SUCCESS;
}
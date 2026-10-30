
#include <stdio.h>
#include "patients.h"

int main(void)
{
    addPatient(101, "Rahul Sharma", 25, "Male", "Fever");
    addPatient(102, "Priya Patel", 30, "Female", "Diabetes");
    addPatient(103, "Amit Khan", 45, "Male", "Blood Pressure");

    printf("\n===== ALL PATIENTS =====\n");
    displayPatients();

    printf("\n===== SEARCH PATIENT =====\n");

    Patient *found = searchPatient(102);

    if (found != NULL)
    {
        printf("Patient found: %s\n", found->name);
    }
    else
    {
        printf("Patient not found.\n");
    }

    printf("\n===== DELETE PATIENT =====\n");

    if (deletePatient(101))
    {
        printf("Patient 101 deleted successfully.\n");
    }
    else
    {
        printf("Patient not found.\n");
    }

    printf("\n===== PATIENTS AFTER DELETION =====\n");
    displayPatients();

    return 0;
}

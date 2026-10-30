
#include <stdio.h>
#include "patients.h"

#define PATIENT_FILE "patients.txt"

int savePatients(void)
{
    FILE *file = fopen(PATIENT_FILE, "w");

    if (file == NULL)
    {
        printf("Error opening patient file for saving.\n");
        return 0;
    }

    Patient *current = getPatientHead();

    while (current != NULL)
    {
        if (fprintf(file, "%d|%s|%d|%s|%s\n",
                    current->id,
                    current->name,
                    current->age,
                    current->gender,
                    current->disease) < 0)
        {
            fclose(file);
            printf("Error writing patient records.\n");
            return 0;
        }

        current = current->next;
    }

    if (fclose(file) != 0)
    {
        printf("Error closing patient file.\n");
        return 0;
    }

    return 1;
}

int loadPatients(void)
{
    FILE *file = fopen(PATIENT_FILE, "r");

    if (file == NULL)
    {
        return 1;
    }

    char line[512];
    int count = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        int id, age;
        char name[100];
        char gender[20];
        char disease[100];

        if (sscanf(line, "%d|%99[^|]|%d|%19[^|]|%99[^\n]",
                   &id, name, &age, gender, disease) == 5)
        {
            if (id > 0 && age > 0 && age <= 120 &&
                searchPatient(id) == NULL)
            {
                addPatient(id, name, age, gender, disease);
                count++;
            }
        }
    }

    fclose(file);

    printf("Loaded %d patient record(s).\n", count);

    return 1;
}

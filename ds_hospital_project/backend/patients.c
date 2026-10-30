
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "patients.h"

static Patient *head = NULL;

void addPatient(int id, const char *name, int age,
                const char *gender, const char *disease)
{
    Patient *newPatient = malloc(sizeof(Patient));

    if (newPatient == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    newPatient->id = id;
    newPatient->age = age;

    snprintf(newPatient->name, sizeof(newPatient->name), "%s", name);
    snprintf(newPatient->gender, sizeof(newPatient->gender), "%s", gender);
    snprintf(newPatient->disease, sizeof(newPatient->disease), "%s", disease);

    newPatient->next = NULL;

    if (head == NULL)
    {
        head = newPatient;
    }
    else
    {
        Patient *current = head;

        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = newPatient;
    }

    printf("Patient added successfully.\n");
}

void displayPatients(void)
{
    Patient *current = head;

    if (current == NULL)
    {
        printf("No patients found.\n");
        return;
    }

    while (current != NULL)
    {
        printf("\nID: %d\n", current->id);
        printf("Name: %s\n", current->name);
        printf("Age: %d\n", current->age);
        printf("Gender: %s\n", current->gender);
        printf("Disease: %s\n", current->disease);

        current = current->next;
    }
}

Patient *searchPatient(int id)
{
    Patient *current = head;

    while (current != NULL)
    {
        if (current->id == id)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

int deletePatient(int id)
{
    Patient *current = head;
    Patient *previous = NULL;

    while (current != NULL && current->id != id)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
    {
        return 0;
    }

    if (previous == NULL)
    {
        head = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    free(current);
    return 1;
}

Patient *getPatientHead(void)
{
    return head;
}

int updatePatient(int id, const char *name, int age,
                  const char *gender, const char *disease)
{
    Patient *patient = searchPatient(id);

    if (patient == NULL)
        return 0;

    if (name == NULL || gender == NULL || disease == NULL ||
        name[0] == '\0' || gender[0] == '\0' ||
        disease[0] == '\0' || age < 1 || age > 120)
        return -1;

    char oldName[100];
    char oldGender[20];
    char oldDisease[100];
    int oldAge = patient->age;

    snprintf(oldName, sizeof(oldName), "%s", patient->name);
    snprintf(oldGender, sizeof(oldGender), "%s", patient->gender);
    snprintf(oldDisease, sizeof(oldDisease), "%s", patient->disease);

    snprintf(patient->name, sizeof(patient->name), "%s", name);
    snprintf(patient->gender, sizeof(patient->gender), "%s", gender);
    snprintf(patient->disease, sizeof(patient->disease), "%s", disease);
    patient->age = age;

    if (!savePatients())
    {
        snprintf(patient->name, sizeof(patient->name), "%s", oldName);
        snprintf(patient->gender, sizeof(patient->gender), "%s", oldGender);
        snprintf(patient->disease, sizeof(patient->disease), "%s", oldDisease);
        patient->age = oldAge;
        return -1;
    }

    return 1;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emergency.h"

static Emergency *head = NULL;
static int nextEmergencyId = 1;

Emergency *getEmergencyHead(void)
{
    return head;
}

Emergency *searchEmergency(int emergencyId)
{
    Emergency *current = head;

    while (current != NULL)
    {
        if (current->emergencyId == emergencyId)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

int addEmergency(
    int emergencyId,
    int patientId,
    const char *patientName,
    const char *emergencyType,
    const char *priority,
    const char *description)
{
    if (emergencyId <= 0 || patientId <= 0 ||
        patientName == NULL || emergencyType == NULL ||
        priority == NULL || description == NULL)
    {
        return 0;
    }

    if (searchEmergency(emergencyId) != NULL)
    {
        return 0;
    }

    Emergency *newNode =
        (Emergency *)malloc(sizeof(Emergency));

    if (newNode == NULL)
    {
        return 0;
    }

    newNode->emergencyId = emergencyId;
    newNode->patientId = patientId;

    snprintf(
        newNode->patientName,
        sizeof(newNode->patientName),
        "%s",
        patientName);

    snprintf(
        newNode->emergencyType,
        sizeof(newNode->emergencyType),
        "%s",
        emergencyType);

    snprintf(
        newNode->priority,
        sizeof(newNode->priority),
        "%s",
        priority);

    snprintf(
        newNode->description,
        sizeof(newNode->description),
        "%s",
        description);

    newNode->next = NULL;

    if (head == NULL)
    {
        head = newNode;
    }
    else
    {
        Emergency *current = head;

        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = newNode;
    }

    if (emergencyId >= nextEmergencyId)
    {
        nextEmergencyId = emergencyId + 1;
    }

    return 1;
}

int deleteEmergency(int emergencyId)
{
    Emergency *current = head;
    Emergency *previous = NULL;

    while (current != NULL &&
           current->emergencyId != emergencyId)
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

void displayEmergencies(void)
{
    Emergency *current = head;

    if (current == NULL)
    {
        printf("No emergency records available.\n");
        return;
    }

    printf("\n========== EMERGENCY RECORDS ==========\n");

    while (current != NULL)
    {
        printf("Emergency ID : %d\n", current->emergencyId);
        printf("Patient ID   : %d\n", current->patientId);
        printf("Patient Name : %s\n", current->patientName);
        printf("Emergency    : %s\n", current->emergencyType);
        printf("Priority     : %s\n", current->priority);
        printf("Description  : %s\n", current->description);
        printf("---------------------------------------\n");

        current = current->next;
    }
}

int saveEmergencies(void)
{
    FILE *file = fopen("emergencies.txt", "w");

    if (file == NULL)
    {
        perror("Could not open emergencies.txt");
        return 0;
    }

    Emergency *current = head;

    while (current != NULL)
    {
        if (fprintf(
                file,
                "%d|%d|%s|%s|%s|%s\n",
                current->emergencyId,
                current->patientId,
                current->patientName,
                current->emergencyType,
                current->priority,
                current->description) < 0)
        {
            fclose(file);
            return 0;
        }

        current = current->next;
    }

    return fclose(file) == 0;
}

int loadEmergencies(void)
{
    FILE *file = fopen("emergencies.txt", "r");

    if (file == NULL)
    {
        return 1;
    }

    char line[512];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        int emergencyId;
        int patientId;

        char patientName[100] = {0};
        char emergencyType[100] = {0};
        char priority[50] = {0};
        char description[200] = {0};

        int fields = sscanf(
            line,
            "%d|%d|%99[^|]|%99[^|]|%49[^|]|%199[^\r\n]",
            &emergencyId,
            &patientId,
            patientName,
            emergencyType,
            priority,
            description);

        if (fields == 6 &&
            searchEmergency(emergencyId) == NULL)
        {
            addEmergency(
                emergencyId,
                patientId,
                patientName,
                emergencyType,
                priority,
                description);
        }
    }

    fclose(file);

    return 1;
}

int getNextEmergencyId(void)
{
    return nextEmergencyId++;
}

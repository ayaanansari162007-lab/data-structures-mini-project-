
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appointments.h"

static Appointment *front = NULL;
static Appointment *rear = NULL;

Appointment *getAppointmentHead(void)
{
    return front;
}

Appointment *searchAppointment(int appointmentId)
{
    Appointment *current = front;

    while (current != NULL)
    {
        if (current->appointmentId == appointmentId)
            return current;

        current = current->next;
    }

    return NULL;
}

int addAppointment(
    int appointmentId,
    int patientId,
    const char *patientName,
    const char *doctorName,
    const char *date,
    const char *time)
{
    if (appointmentId <= 0 ||
        patientId <= 0 ||
        patientName == NULL ||
        doctorName == NULL ||
        date == NULL ||
        time == NULL ||
        searchAppointment(appointmentId) != NULL)
    {
        return 0;
    }

    Appointment *node =
        (Appointment *)malloc(sizeof(Appointment));

    if (node == NULL)
        return 0;

    memset(node, 0, sizeof(Appointment));

    node->appointmentId = appointmentId;
    node->patientId = patientId;

    snprintf(node->patientName,
             sizeof(node->patientName),
             "%s", patientName);

    snprintf(node->doctorName,
             sizeof(node->doctorName),
             "%s", doctorName);

    snprintf(node->date,
             sizeof(node->date),
             "%s", date);

    snprintf(node->time,
             sizeof(node->time),
             "%s", time);

    node->next = NULL;

    if (rear == NULL)
    {
        front = rear = node;
    }
    else
    {
        rear->next = node;
        rear = node;
    }

    return 1;
}

int deleteAppointment(int appointmentId)
{
    Appointment *current = front;
    Appointment *previous = NULL;

    while (current != NULL &&
           current->appointmentId != appointmentId)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
        return 0;

    if (previous == NULL)
        front = current->next;
    else
        previous->next = current->next;

    if (rear == current)
        rear = previous;

    free(current);

    return 1;
}

void displayAppointments(void)
{
    Appointment *current = front;

    if (current == NULL)
    {
        printf("No appointments in the queue.\n");
        return;
    }

    printf("\nAppointments (FIFO Queue):\n");

    while (current != NULL)
    {
        printf(
            "Appointment ID: %d | Patient ID: %d | "
            "Patient: %s | Doctor: %s | Date: %s | Time: %s\n",
            current->appointmentId,
            current->patientId,
            current->patientName,
            current->doctorName,
            current->date,
            current->time);

        current = current->next;
    }
}

int saveAppointments(void)
{
    FILE *file = fopen("appointments.txt", "w");

    if (file == NULL)
        return 0;

    Appointment *current = front;

    while (current != NULL)
    {
        if (fprintf(
                file,
                "%d|%d|%s|%s|%s|%s\n",
                current->appointmentId,
                current->patientId,
                current->patientName,
                current->doctorName,
                current->date,
                current->time) < 0)
        {
            fclose(file);
            return 0;
        }

        current = current->next;
    }

    return fclose(file) == 0;
}

int loadAppointments(void)
{
    FILE *file = fopen("appointments.txt", "r");

    if (file == NULL)
        return 1;

    char line[512];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        int appointmentId;
        int patientId;

        char patientName[100] = {0};
        char doctorName[100] = {0};
        char date[20] = {0};
        char time[20] = {0};

        int fields = sscanf(
            line,
            "%d|%d|%99[^|]|%99[^|]|%19[^|]|%19[^\r\n]",
            &appointmentId,
            &patientId,
            patientName,
            doctorName,
            date,
            time);

        if (fields == 6 &&
            searchAppointment(appointmentId) == NULL)
        {
            addAppointment(
                appointmentId,
                patientId,
                patientName,
                doctorName,
                date,
                time);
        }
    }

    fclose(file);

    return 1;
}

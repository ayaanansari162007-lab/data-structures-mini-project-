
#ifndef APPOINTMENTS_H
#define APPOINTMENTS_H

typedef struct Appointment
{
    int appointmentId;
    int patientId;

    char patientName[100];
    char doctorName[100];
    char date[20];
    char time[20];

    struct Appointment *next;
} Appointment;

int addAppointment(
    int appointmentId,
    int patientId,
    const char *patientName,
    const char *doctorName,
    const char *date,
    const char *time);

Appointment *getAppointmentHead(void);

Appointment *searchAppointment(int appointmentId);

int deleteAppointment(int appointmentId);

void displayAppointments(void);

int saveAppointments(void);

int loadAppointments(void);

#endif


#ifndef PATIENTS_H
#define PATIENTS_H

typedef struct Patient
{
    int id;
    char name[100];
    int age;
    char gender[20];
    char disease[100];
    struct Patient *next;
} Patient;

void addPatient(int id, const char *name, int age,
                const char *gender, const char *disease);

void displayPatients(void);
Patient *searchPatient(int id);
int deletePatient(int id);
Patient *getPatientHead(void);
int savePatients(void);
int loadPatients(void);
int updatePatient(int id, const char *name, int age,
                  const char *gender, const char *disease);
#endif

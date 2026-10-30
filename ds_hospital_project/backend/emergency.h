#ifndef EMERGENCY_H
#define EMERGENCY_H

typedef struct Emergency
{
    int emergencyId;
    int patientId;

    char patientName[100];
    char emergencyType[100];
    char priority[50];
    char description[200];

    struct Emergency *next;

} Emergency;

Emergency *getEmergencyHead(void);

Emergency *searchEmergency(int emergencyId);

int addEmergency(
    int emergencyId,
    int patientId,
    const char *patientName,
    const char *emergencyType,
    const char *priority,
    const char *description);

int deleteEmergency(int emergencyId);

void displayEmergencies(void);

int saveEmergencies(void);

int loadEmergencies(void);

int getNextEmergencyId(void);

#endif
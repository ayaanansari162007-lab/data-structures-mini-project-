#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include "patients.h"
#include "appointments.h"

#define PORT 8080
#define BUFFER_SIZE 16384
#define RESPONSE_SIZE 32768

static void sendResponse(SOCKET client, int status,
                         const char *statusText, const char *body)
{
    char response[RESPONSE_SIZE];
    int length = snprintf(
        response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "Content-Length: %lu\r\n\r\n%s",
        status, statusText, (unsigned long)strlen(body), body);

    if (length > 0 && (size_t)length < sizeof(response))
        send(client, response, length, 0);
}

static void appendJsonString(char *dest, size_t size, const char *src)
{
    size_t used = strlen(dest);
    size_t i;

    if (size == 0 || used >= size || src == NULL)
        return;

    for (i = 0; src[i] != '\0'; i++)
    {
        char escaped[2];
        size_t count = 1;

        if (src[i] == '"' || src[i] == '\\')
        {
            escaped[0] = '\\';
            escaped[1] = src[i];
            count = 2;
        }
        else if ((unsigned char)src[i] < 32)
        {
            continue;
        }
        else
        {
            escaped[0] = src[i];
        }

        if (used + count >= size)
            return;

        dest[used++] = escaped[0];
        if (count == 2)
            dest[used++] = escaped[1];
        dest[used] = '\0';
    }
}

static char *getRequestBody(char *request)
{
    char *body = strstr(request, "\r\n\r\n");
    return body == NULL ? NULL : body + 4;
}

/* Extract a JSON string field into dest. Returns 1 when found. */
static int getJsonString(const char *body, const char *key,
                         char *dest, size_t destSize)
{
    char pattern[80];
    const char *p;
    size_t out = 0;

    if (body == NULL || dest == NULL || destSize == 0)
        return 0;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(body, pattern);
    if (p == NULL)
        return 0;

    p += strlen(pattern);
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != ':')
        return 0;
    p++;
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '"')
        return 0;
    p++;

    while (*p && *p != '"')
    {
        char ch = *p++;
        if (ch == '\\' && *p)
        {
            ch = *p++;
            if (ch == 'n')
                ch = '\n';
            else if (ch == 'r')
                ch = '\r';
            else if (ch == 't')
                ch = '\t';
        }
        if (out + 1 < destSize)
            dest[out++] = ch;
    }

    if (*p != '"')
        return 0;
    dest[out] = '\0';
    return 1;
}

/* Extract an integer JSON field. Returns 1 when found. */
static int getJsonInt(const char *body, const char *key, int *value)
{
    char pattern[80];
    const char *p;
    char *end;
    long parsed;

    if (body == NULL || value == NULL)
        return 0;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(body, pattern);
    if (p == NULL)
        return 0;

    p += strlen(pattern);
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != ':')
        return 0;
    p++;
    while (*p && isspace((unsigned char)*p))
        p++;

    parsed = strtol(p, &end, 10);
    if (end == p || parsed < 0 || parsed > 2147483647L)
        return 0;

    *value = (int)parsed;
    return 1;
}

static void handleGetPatients(SOCKET client)
{
    Patient *current = getPatientHead();
    char json[BUFFER_SIZE] = "{\"patients\":[";
    int first = 1;

    while (current != NULL)
    {
        char item[1024] = {0};
        char name[256] = {0};
        char gender[64] = {0};
        char disease[256] = {0};
        int itemLength;

        appendJsonString(name, sizeof(name), current->name);
        appendJsonString(gender, sizeof(gender), current->gender);
        appendJsonString(disease, sizeof(disease), current->disease);

        itemLength = snprintf(
            item, sizeof(item),
            "%s{\"id\":%d,\"name\":\"%s\",\"age\":%d,\"gender\":\"%s\",\"disease\":\"%s\"}",
            first ? "" : ",", current->id, name, current->age,
            gender, disease);

        if (itemLength < 0 || (size_t)itemLength >= sizeof(item) ||
            strlen(json) + (size_t)itemLength + 3 >= sizeof(json))
        {
            sendResponse(client, 500, "Internal Server Error",
                         "{\"error\":\"Patient list is too large.\"}");
            return;
        }

        strcat(json, item);
        first = 0;
        current = current->next;
    }

    strcat(json, "]}");
    sendResponse(client, 200, "OK", json);
}

static void handleGetAppointments(SOCKET client)
{
    Appointment *current = getAppointmentHead();
    char json[BUFFER_SIZE] = "{\"appointments\":[";
    int first = 1;

    while (current != NULL)
    {
        char item[1400] = {0};
        char patientName[256] = {0};
        char doctorName[256] = {0};
        char date[64] = {0};
        char time[64] = {0};
        int itemLength;

        appendJsonString(patientName, sizeof(patientName), current->patientName);
        appendJsonString(doctorName, sizeof(doctorName), current->doctorName);
        appendJsonString(date, sizeof(date), current->date);
        appendJsonString(time, sizeof(time), current->time);

        itemLength = snprintf(
            item, sizeof(item),
            "%s{\"appointmentId\":%d,\"patientId\":%d,\"patientName\":\"%s\",\"doctorName\":\"%s\",\"date\":\"%s\",\"time\":\"%s\"}",
            first ? "" : ",", current->appointmentId, current->patientId,
            patientName, doctorName, date, time);

        if (itemLength < 0 || (size_t)itemLength >= sizeof(item) ||
            strlen(json) + (size_t)itemLength + 3 >= sizeof(json))
        {
            sendResponse(client, 500, "Internal Server Error",
                         "{\"error\":\"Appointment list is too large.\"}");
            return;
        }

        strcat(json, item);
        first = 0;
        current = current->next;
    }

    strcat(json, "]}");
    sendResponse(client, 200, "OK", json);
}

static void handleAddAppointment(SOCKET client, const char *body)
{
    int appointmentId = 0;
    int patientId = 0;
    char patientName[100] = {0};
    char doctorName[100] = {0};
    char date[20] = {0};
    char time[20] = {0};
    Patient *patient;

    if (!getJsonInt(body, "appointmentId", &appointmentId) ||
        !getJsonInt(body, "patientId", &patientId) ||
        !getJsonString(body, "patientName", patientName, sizeof(patientName)) ||
        !getJsonString(body, "doctorName", doctorName, sizeof(doctorName)) ||
        !getJsonString(body, "date", date, sizeof(date)) ||
        !getJsonString(body, "time", time, sizeof(time)) ||
        appointmentId <= 0 || patientId <= 0 ||
        patientName[0] == '\0' || doctorName[0] == '\0' ||
        date[0] == '\0' || time[0] == '\0')
    {
        sendResponse(client, 400, "Bad Request",
                     "{\"error\":\"Missing or invalid appointment fields. Use numeric appointmentId and patientId.\"}");
        return;
    }

    if (searchAppointment(appointmentId) != NULL)
    {
        sendResponse(client, 409, "Conflict",
                     "{\"error\":\"Appointment ID already exists.\"}");
        return;
    }

    patient = searchPatient(patientId);
    if (patient == NULL)
    {
        sendResponse(client, 404, "Not Found",
                     "{\"error\":\"Selected patient does not exist. Register the patient first.\"}");
        return;
    }

    /* Use the authoritative patient name stored in the C patient list. */
    snprintf(patientName, sizeof(patientName), "%s", patient->name);

    if (!addAppointment(appointmentId, patientId, patientName,
                        doctorName, date, time))
    {
        sendResponse(client, 500, "Internal Server Error",
                     "{\"error\":\"Could not add appointment to the queue.\"}");
        return;
    }

    if (!saveAppointments())
    {
        deleteAppointment(appointmentId);
        sendResponse(client, 500, "Internal Server Error",
                     "{\"error\":\"Could not save appointment records. Check folder permissions.\"}");
        return;
    }

    sendResponse(client, 201, "Created",
                 "{\"status\":\"success\",\"message\":\"Appointment added successfully.\"}");
}

static void handleDeleteAppointment(SOCKET client, const char *body)
{
    int appointmentId = 0;

    if (!getJsonInt(body, "appointmentId", &appointmentId) || appointmentId <= 0)
    {
        sendResponse(client, 400, "Bad Request",
                     "{\"error\":\"Invalid appointment ID.\"}");
        return;
    }

    if (!deleteAppointment(appointmentId))
    {
        sendResponse(client, 404, "Not Found",
                     "{\"error\":\"Appointment not found.\"}");
        return;
    }

    if (!saveAppointments())
    {
        sendResponse(client, 500, "Internal Server Error",
                     "{\"error\":\"Appointment cancelled in memory, but records could not be saved.\"}");
        return;
    }

    sendResponse(client, 200, "OK",
                 "{\"status\":\"success\",\"message\":\"Appointment cancelled successfully.\"}");
}

static void handleRequest(SOCKET client, char *request)
{
    char method[16] = {0};
    char path[256] = {0};
    char *body;

    if (sscanf(request, "%15s %255s", method, path) != 2)
    {
        sendResponse(client, 400, "Bad Request",
                     "{\"error\":\"Invalid request.\"}");
        return;
    }

    if (strcmp(method, "OPTIONS") == 0)
    {
        sendResponse(client, 204, "No Content", "");
        return;
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/health") == 0)
    {
        sendResponse(client, 200, "OK",
                     "{\"status\":\"success\",\"message\":\"CarePlus C backend is running!\"}");
        return;
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/patients") == 0)
    {
        handleGetPatients(client);
        return;
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/appointments") == 0)
    {
        handleGetAppointments(client);
        return;
    }

    body = getRequestBody(request);
    if (body == NULL &&
        (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0 ||
         strcmp(method, "DELETE") == 0))
    {
        sendResponse(client, 400, "Bad Request",
                     "{\"error\":\"Invalid request body.\"}");
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/appointments") == 0)
    {
        handleAddAppointment(client, body);
        return;
    }

    if (strcmp(method, "DELETE") == 0 && strcmp(path, "/api/appointments") == 0)
    {
        handleDeleteAppointment(client, body);
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/patients") == 0)
    {
        int id = 0;
        int age = 0;
        char name[100] = {0};
        char gender[20] = {0};
        char disease[100] = {0};

        if (!getJsonInt(body, "id", &id) ||
            !getJsonInt(body, "age", &age) ||
            !getJsonString(body, "name", name, sizeof(name)) ||
            !getJsonString(body, "gender", gender, sizeof(gender)) ||
            !getJsonString(body, "disease", disease, sizeof(disease)) ||
            id <= 0 || age <= 0 || age > 120 || name[0] == '\0' ||
            gender[0] == '\0' || disease[0] == '\0')
        {
            sendResponse(client, 400, "Bad Request",
                         "{\"error\":\"Missing or invalid patient fields.\"}");
            return;
        }

        if (searchPatient(id) != NULL)
        {
            sendResponse(client, 409, "Conflict",
                         "{\"error\":\"Patient ID already exists.\"}");
            return;
        }

        addPatient(id, name, age, gender, disease);
        if (!savePatients())
        {
            deletePatient(id);
            sendResponse(client, 500, "Internal Server Error",
                         "{\"error\":\"Could not save patient record.\"}");
            return;
        }

        sendResponse(client, 201, "Created",
                     "{\"status\":\"success\",\"message\":\"Patient registered successfully.\"}");
        return;
    }

    if (strcmp(method, "PUT") == 0 && strcmp(path, "/api/patients") == 0)
    {
        int id = 0;
        int age = 0;
        char name[100] = {0};
        char gender[20] = {0};
        char disease[100] = {0};

        if (!getJsonInt(body, "id", &id) ||
            !getJsonInt(body, "age", &age) ||
            !getJsonString(body, "name", name, sizeof(name)) ||
            !getJsonString(body, "gender", gender, sizeof(gender)) ||
            !getJsonString(body, "disease", disease, sizeof(disease)))
        {
            sendResponse(client, 400, "Bad Request",
                         "{\"error\":\"Missing or invalid patient fields.\"}");
            return;
        }

        {
            int updateResult = updatePatient(id, name, age, gender, disease);
            if (updateResult == 0)
            {
                sendResponse(client, 404, "Not Found",
                             "{\"error\":\"Patient not found.\"}");
                return;
            }
            if (updateResult < 0)
            {
                sendResponse(client, 400, "Bad Request",
                             "{\"error\":\"Could not update patient.\"}");
                return;
            }
        }

        if (!savePatients())
        {
            sendResponse(client, 500, "Internal Server Error",
                         "{\"error\":\"Patient updated, but records could not be saved.\"}");
            return;
        }

        sendResponse(client, 200, "OK",
                     "{\"status\":\"success\",\"message\":\"Patient updated successfully.\"}");
        return;
    }

    if (strcmp(method, "DELETE") == 0 && strcmp(path, "/api/patients") == 0)
    {
        int id = 0;
        if (!getJsonInt(body, "id", &id) || id <= 0)
        {
            sendResponse(client, 400, "Bad Request",
                         "{\"error\":\"Invalid patient ID.\"}");
            return;
        }

        if (!deletePatient(id))
        {
            sendResponse(client, 404, "Not Found",
                         "{\"error\":\"Patient not found.\"}");
            return;
        }

        if (!savePatients())
        {
            sendResponse(client, 500, "Internal Server Error",
                         "{\"error\":\"Patient deleted in memory, but records could not be saved.\"}");
            return;
        }

        sendResponse(client, 200, "OK",
                     "{\"status\":\"success\",\"message\":\"Patient deleted successfully.\"}");
        return;
    }

    sendResponse(client, 404, "Not Found",
                 "{\"error\":\"Endpoint not found.\"}");
}

int main(void)
{
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientLength;
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Could not bind port %d. Stop any old server first.\n", PORT);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR)
    {
        printf("Listen failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (!loadPatients())
        printf("Warning: Could not load patient records.\n");

    if (!loadAppointments())
        printf("Warning: Could not load appointment records.\n");

    printf("CarePlus backend: http://127.0.0.1:%d\n", PORT);
    printf("Patient endpoint: http://127.0.0.1:%d/api/patients\n", PORT);
    printf("Appointment endpoint: http://127.0.0.1:%d/api/appointments\n", PORT);
    printf("Press Ctrl+C to stop.\n");

    while (1)
    {
        clientLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket,
                              (struct sockaddr *)&clientAddress,
                              &clientLength);

        if (clientSocket == INVALID_SOCKET)
            continue;

        memset(buffer, 0, sizeof(buffer));
        {
            int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (received > 0)
            {
                buffer[received] = '\0';
                handleRequest(clientSocket, buffer);
            }
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
    
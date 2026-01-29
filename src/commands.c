#include "commands.h"

void sendResponse(int clientSock, const char *cmd, int status, const char *extra) {
    char response[2048]; // ปรับขนาดตามความเหมาะสม
    if (extra != NULL && strlen(extra) > 0) {
        snprintf(response, sizeof(response), "%s,%d,%s\n", cmd, status, extra);
    } else {
        snprintf(response, sizeof(response), "%s,%d\n", cmd, status);
    }
    send(clientSock, response, strlen(response), 0);
}
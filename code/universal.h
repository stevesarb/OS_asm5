#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

char* append_data(char* wholeMSG, char buffer[]) {
    char* temp = NULL;
    if (wholeMSG != NULL) {
        // printf("WHOLEMSG != NULL\n");
        temp = calloc(strlen(wholeMSG) + strlen(buffer) + 1, sizeof(char));
        memset(temp, '\0', strlen(wholeMSG) + strlen(buffer) + 1);
        strcat(temp, wholeMSG);
        // printf("ABOUT TO FREE!\n");
        free(wholeMSG);
    }
    else {
        // printf("WHOLEMSG == NULL\n");
        temp = calloc(strlen(buffer) + 1, sizeof(char));
        memset(temp, '\0', strlen(buffer) + 1);
    }

    // printf("ABOUT TO STRCAT(temp, buffer)!\n");
    strcat(temp, buffer);
    wholeMSG = temp;
    temp = NULL;

    return wholeMSG;
}

#endif
#ifndef CLIENT_H
#define CLIENT_H

int check_file(FILE* file, char** textPtr) {
    size_t len;
    ssize_t nread;
    nread = getline(textPtr, &len, file);
    (*textPtr)[nread - 1] = '\0'; // replace newline with null terminator

    // printf("text: %s [\n] nread: %d\n", *textPtr, nread);
    // printf("strlen(text): %d\n", strlen(*textPtr));

    char* text = *textPtr;
    int asciiVal, i = 0;

    // check text for invalid characters
    while (i < strlen(text)) {
        asciiVal = text[i];

        // printf("i: %d\ttext[i]: %c\tasciiVal: %d\n", i, text[i], asciiVal);

        if ( ((asciiVal < 65) || (asciiVal > 90)) && (asciiVal != 32) )
            return -1;
        ++i;
    }

    char* textWithCC = calloc(strlen(text) + 1, sizeof(char));
    memset(textWithCC, '\0', strlen(textWithCC));
    strcpy(textWithCC, text);
    textWithCC[strlen(text)] = '#';
    // printf("textWithCC: %s/\tlength: %d/\n", textWithCC, strlen(textWithCC));

    free(text);
    *textPtr = textWithCC;

    return 0;
}

char* get_data(int socketFD) {
    char* data = NULL;
    char buffer[256];
    int charsRead;

    do {
        memset(buffer, '\0', 256);
        charsRead = recv(socketFD, buffer, 255, 0); // read server's response
        if (charsRead < 0) error("CLIENT: ERROR reading from socket\n");

        // add buffer to data
        data = append_data(data, buffer);

    } while (strstr(data, "!") == NULL);

    data[strlen(data) - 1] = '\0'; // replace CC with terminator

    // printf("CLIENT, data: %s\n", data);

    return data;
}

void check_server(int socketFD, char code) {
    int charsRead;
    char buffer[10];
    memset(buffer, '\0', 10);

    // get code from server
    charsRead = recv(socketFD, buffer, 9, 0);
    if (charsRead < 0) error("ERROR reading from socket");

    // printf("CLIENT check_server: %s\n", buffer);

    // check that the connection is with the encryption server
    if (buffer[0] != code) {
        // if not connected to correct server
        // send 'q' back to server so that it knows to quit processing
        charsRead = send(socketFD, "q", 1, 0);
        if (charsRead < 0) error("SERVER: ERROR writing to socket");
        close(socketFD);
        if (code == 'e')
            perror("enc_client: cannot use dec_server\n");
        if (code == 'd')
            perror("dec_client: cannot use enc_server\n");
        exit(2);
    }

    // if server is correct, send confirmation back to server
    charsRead = send(socketFD, &code, 1, 0);
    sleep(1); // sleep for 1s so that the server only reads the 'e' (and not the message that will be sent as soon as this function ends as well)
}

#endif
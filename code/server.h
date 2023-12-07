#ifndef SERVER_H
#define SERVER_H

void push_back(pid_t* pid_arr, pid_t pid) {
    int i = 0;
    while (i < 5) {
        if (pid_arr[i] == -5) {
            pid_arr[i] = pid;
            break;
        }
        ++i;
    }
}

pid_t* check_children(pid_t* pid_arr, int* num_children) {
    pid_t termChild = -5;
    int childExitMethod = -5;
    int i = 0;
    pid_t* new_arr = calloc(5, sizeof(pid_t));
    memset(new_arr, -5, 5);
    while (i < 5) {
        termChild = waitpid(pid_arr[i], &childExitMethod, WNOHANG);

        // if child has not terminated
        if (termChild == 0) 
            push_back(new_arr, pid_arr[i]);
        else 
            (*num_children)--;
        
        ++i;
    }
    free(pid_arr);

    return new_arr;
}

char* get_data(int establishedConnectionFD) {
    char* data = NULL;
    char* hashtag = NULL;
    char buffer[256];
    int charsRead;
    
    while (1) {
        // read next 255 characters of message
        memset(buffer, '\0', 256);
        charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
        if (charsRead < 0) error("ERROR reading from socket");
        
        // add buffer to data
        data = append_data(data, buffer);

        // check for transmission end signal (2 lone '#' symbols: 
        // one at the end of the message, one at the end of the key)
        hashtag = strstr(data, "#");
        if (hashtag != NULL) {
            // first # found, search for the second one
            if (strstr(hashtag + 1, "#") != NULL) // begin searching one char to the right of the first #
                break;
        }
    }

    data[strlen(data) - 1] = '\0'; // replace trailing # with null terminator

    // printf("SERVER, data: %s\n", data);

    // data is a string containing both the msg and key, seperated by a # symbol
    return data; 
}

void check_client(int establishedConnectionFD, char code) {
    int charsRead;
    char buffer[10];
    memset(buffer, '\0', 10);

    // send 'd' for decryption server
    charsRead = send(establishedConnectionFD, &code, 1, 0); 
    if (charsRead < 0) error("SERVER: ERROR writing to socket");

    // get reponse from client
    charsRead = recv(establishedConnectionFD, buffer, 9, 0);
    if (charsRead < 0) error("ERROR reading from socket");

    // printf("SERVER check_client: %s\n", buffer);

    // if connection is with decryption client, client will send back 'd'
    // if connection is not with a decryption client, close the socket and exit the child process
    if (buffer[0] != code) {
        close(establishedConnectionFD);
        exit(0);
    }
}

#endif
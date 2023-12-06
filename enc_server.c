#include "functions.h"

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void push_back(pid_t*, pid_t);
pid_t* check_children(pid_t*);
char* get_data(int);
// char* append_data(char* wholeMSG, char buffer[]);
char* encrypt_msg(char*, char*);

int main(int argc, char *argv[]) {
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
    socklen_t sizeOfClientInfo;
    struct sockaddr_in serverAddress, clientAddress;
    
    if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
    
    // Set up the address struct for this process (the server)
    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string 
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
    
    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket 
    if (listenSocketFD < 0) error("ERROR opening socket");

    // Enable the socket to begin listening
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
        error("ERROR on binding");
    listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

    char* data = NULL;
    char* msg = NULL;
    char* key = NULL;
    char* cipherText = NULL;
    pid_t spawnpid = -5;
    pid_t* pid_arr = calloc(5, sizeof(pid_t));
    while (1) {
        // check on child processes
        pid_arr = check_children(pid_arr);

        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept 
        if (establishedConnectionFD < 0) error("ERROR on accept");
        printf("SERVER: Connected Client at port %d\n", ntohs(clientAddress.sin_port));

        // FORK HAPPENS HERE
        spawnpid = fork();
        if (spawnpid < 0) error("fork() failed\n");

        // child
        else if (spawnpid == 0) {
            // Get the message from the client
            data = get_data(establishedConnectionFD);
            key = strstr(data, "#") + 1;
            *(key - 1) = '\0'; // replace # that separates message and key with null terminator
            msg = data;
            /* NOTE: msg and key are just pointers that point to the same block of memory, just different segments of it */
            
            printf("SERVER: msg received from client: \"%s\"\n", msg);
            printf("SERVER: key received from client: \"%s\"\n", key);
            
            // encrypt message using provided key
            cipherText = encrypt_msg(msg, key);

            // Send a Success message back to the client
            charsRead = send(establishedConnectionFD, cipherText, strlen(cipherText), 0); // Send encrypted message back
            if (charsRead < 0) error("SERVER: ERROR writing to socket");
            close(establishedConnectionFD); // Close the existing socket which is connected to the client
            exit(0);
        }

        // parent
        else {
            push_back(pid_arr, spawnpid);
        }
    }

    close(listenSocketFD); // Close the listening socket
    return 0;
}

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

pid_t* check_children(pid_t* pid_arr) {
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

    // data is a string containing both the msg and key, seperated by a # symbol
    return data; 
}

// char* append_data(char* wholeMSG, char buffer[]) {
//     char* temp = NULL;
//     if (wholeMSG != NULL) {
//         // printf("WHOLEMSG != NULL\n");
//         temp = calloc(strlen(wholeMSG) + strlen(buffer) + 1, sizeof(char));
//         memset(temp, '\0', strlen(wholeMSG) + strlen(buffer) + 1);
//         strcat(temp, wholeMSG);
//         // printf("ABOUT TO FREE!\n");
//         free(wholeMSG);
//     }
//     else {
//         // printf("WHOLEMSG == NULL\n");
//         temp = calloc(strlen(buffer) + 1, sizeof(char));
//         memset(temp, '\0', strlen(buffer) + 1);
//     }

//     // printf("ABOUT TO STRCAT(temp, buffer)!\n");
//     strcat(temp, buffer);
//     wholeMSG = temp;
//     temp = NULL;

//     return wholeMSG;
// }

char* encrypt_msg(char* msg, char* key) {
    if (strlen(key) < strlen(msg)) error("SERVER: key is too short!\n");

    char* cipherText = calloc(strlen(msg) + 2, sizeof(char));
    memset(cipherText, '\0', strlen(msg) + 2);

    int i = 0, msgCharVal, keyCharVal;
    while (i < strlen(msg)) { // loop through msg string
        if (msg[i] == ' ')
            msgCharVal = msg[i] - 6; // treat spaces as a 27th character (numeric value of 26)
        else
            msgCharVal = msg[i] - 65; // assign all other characters a numeric value 0(A)-25(Z)

        // repeat for key
        if (key[i] == ' ')
            keyCharVal = key[i] - 6;
        else
            keyCharVal = key[i] - 65;
        
        cipherText[i] = ((msgCharVal + keyCharVal) % 27) + 65; // encryption

        // encryption doesn't handle spaces properly, so we must assign them manually
        if (cipherText[i] == '[')
            cipherText[i] = ' ';

        ++i;
    }

    cipherText[i] = '!'; // add control code to end of string

    return cipherText;
}
#include <netdb.h>
#include <fcntl.h>
#include "functions.h"

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
int check_file(FILE*, char**);
char* get_data(int);
void check_server(int);

int main(int argc, char *argv[])
{
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[256];
    char* plainText = NULL;

    if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

    // check text and key files for any bad character and for valid lengths
    char* key = NULL;
    FILE* keyFile = fopen(argv[2], "r"); // open file
    if (keyFile == NULL) error("Couldn't open key file\n"); 
    int keyValid = check_file(keyFile, &key); fclose(keyFile);
    // printf("key: %s\tkeyValid: %d\n", key, keyValid);

    char* msg = NULL;
    FILE* msgFile = fopen(argv[1], "r");
    if (msgFile == NULL) error("Couldn't open msg file\n"); //{perror("Couldn't open msg file\n"); exit(1);}
    int msgValid = check_file(msgFile, &msg); fclose(msgFile);
    // printf("msg: %s\tmsgValid: %d\n", msg, msgValid); exit(0);

    if (msgValid < 0) error("Message had invalid characters!\n");
    if (keyValid < 0) error("Key had invalid characters!\n"); 
    if (strlen(key) < strlen(msg)) error("Key is shorter than the message!\n");

    
    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) error("CLIENT: ERROR opening socket");
    
    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to addy
        error("CLIENT: ERROR connecting");

    
    // check that the client is connected to the encryption server
    check_server(socketFD);
    
    // Send message to server
    charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(msg)) perror("CLIENT: WARNING: Not all data written to socket!\n");

    // send key to server
    charsWritten = send(socketFD, key, strlen(key), 0);
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(key)) perror("CLIENT: WARNING: Not all data written to socket!\n");
    
    // Get return message from server
    plainText = get_data(socketFD);
    printf("%s\n", plainText);
    
    close(socketFD); // Close the socket
    return 0;
}

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

    printf("CLIENT, data: %s\n", data);

    return data;
}

void check_server(int socketFD) {
    int charsRead;
    char buffer[10];
    memset(buffer, '\0', 10);

    // get code from server
    charsRead = recv(socketFD, buffer, 9, 0);
    if (charsRead < 0) error("ERROR reading from socket");

    printf("CLIENT check_server: %s\n", buffer);

    // check that the connection is with the decryption server
    if (buffer[0] != 'd') {
        // if not connected to correct server
        // send 'q' back to server so that it knows to quit processing
        charsRead = send(socketFD, "q", 1, 0);
        if (charsRead < 0) error("SERVER: ERROR writing to socket");
        close(socketFD);
        perror("CLIENT: tried to connect to wrong server\n");
        exit(2);
    }

    // if server is correct, send confirmation back to server
    charsRead = send(socketFD, "d", 1, 0);
    sleep(1); // sleep for 1s so that the server only reads the 'd' (and not the message that will be sent as soon as this function ends as well)
}
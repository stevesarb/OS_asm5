#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
int check_file(FILE*, char**);

int main(int argc, char *argv[])
{
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[256];

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
    
    // Get input message from user
    // printf("CLIENT: Enter text to send to the server, and then hit enter: ");
    // memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    // fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
    // buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
    
    // Send message to server
    charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(msg)) printf("CLIENT: WARNING: Not all data written to socket!\n");
    
    exit(0);

    // Get return message from server
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
    
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